#include "scs/Synthesis/Exact/arena.h"

#include <algorithm>
#include <deque>
#include <limits>
#include <numeric>
#include <ranges>
#include <set>
#include <stdexcept>

#include "scs/FirstOrderLogic/evaluator.h"
#include "scs/FirstOrderLogic/formula_utils.h"
#include "scs/Synthesis/Topology/Incremental/incremental.h"

namespace scs {
namespace {

	struct BoundTransition {
		CompoundAction action;
		FirstOrderAssignment target_bindings;
	};

	struct BoundFacilityTransition {
		JointAction action;
		std::vector<FirstOrderAssignment> target_bindings;
	};

	bool ObjectLess(const Object& lhs, const Object& rhs) {
		if (lhs.kind() != rhs.kind()) return lhs.kind() < rhs.kind();
		return lhs.name() < rhs.name();
	}

	std::vector<Object> Ordered(const ObjectSet& objects) {
		std::vector<Object> result(objects.begin(), objects.end());
		std::ranges::sort(result, ObjectLess);
		return result;
	}

	void AddActionObjects(ObjectSet& objects, const CompoundAction& action) {
		AddGroundActionObjects(objects, action);
	}

	void AddLabelObjects(ObjectSet& objects, const ArenaLabel& label) {
		if (const auto* request = std::get_if<CompoundAction>(&label)) {
			AddActionObjects(objects, *request);
		} else if (const auto* joint = std::get_if<JointAction>(&label)) {
			AddActionObjects(objects, joint->Flatten());
		}
	}

	Action Ground(const Action& action, const FirstOrderAssignment& assignment) {
		Action result = action;
		for (auto& term : result.terms) {
			if (const auto* variable = std::get_if<Variable>(&term)) {
				term = assignment.GetObject(*variable);
			}
		}
		return result;
	}

	CompoundAction Ground(const CompoundAction& action, const FirstOrderAssignment& assignment) {
		CompoundAction result;
		for (const auto& component : action.Actions()) result.AppendAction(Ground(component, assignment));
		return result;
	}

	Object RenameObject(const Object& object, const ObjectRenaming& renaming) {
		if (object.IsRigid()) return object;
		const auto found = std::ranges::find(renaming, object, [](const auto& pair) { return pair.first; });
		return found == renaming.end() ? object : found->second;
	}

	FirstOrderAssignment RenameAssignment(const FirstOrderAssignment& assignment,
		const ObjectRenaming& renaming) {
		FirstOrderAssignment result;
		for (const auto& [variable, value] : assignment.Values()) {
			if (const auto* object = std::get_if<Object>(&value)) {
				result.Set(variable, RenameObject(*object, renaming));
			} else if (const auto* action = std::get_if<Action>(&value)) {
				CompoundAction one{*action};
				const auto renamed = std::get<CompoundAction>(RenameLabel(ArenaLabel{one}, renaming));
				result.Set(variable, renamed.Actions().front());
			} else if (const auto* compound = std::get_if<CompoundAction>(&value)) {
				result.Set(variable, std::get<CompoundAction>(RenameLabel(ArenaLabel{*compound}, renaming)));
			} else {
				result.Set(variable, std::get<bool>(value));
			}
		}
		return result;
	}

	Situation RenameInterpretation(const Situation& state, const ObjectRenaming& renaming) {
		Situation result;
		for (const auto& [name, fluent] : state.Fluents()) {
			RelationalFluent renamed{fluent.Arity()};
			for (const auto& tuple : fluent.TrueTuples()) {
				std::vector<Object> values;
				values.reserve(tuple.size());
				for (const auto& object : tuple) values.emplace_back(RenameObject(object, renaming));
				renamed.AddValuation(std::move(values), true);
			}
			result.AddFluent(name, std::move(renamed));
		}
		return result;
	}

	bool Holds(const Formula& formula, const Situation& state, const BasicActionTheory& bat,
		const FirstOrderAssignment& assignment, const ObjectSet& objects,
		DomainSemantics semantics) {
		Domain domain{state, bat, objects};
		domain.semantics = semantics;
		return EvaluateFormula(formula, std::move(domain), assignment);
	}

	std::vector<FirstOrderAssignment> EnumerateAssignments(
		const std::vector<Variable>& variables, const FirstOrderAssignment& source,
		const ObjectSet& objects, const ObjectSet& reserve, bool equality_types,
		ArenaStatistics& statistics) {
		std::vector<Object> persistent;
		std::vector<Object> fresh;
		for (const auto& object : Ordered(objects)) {
			(reserve.contains(object) ? fresh : persistent).push_back(object);
		}

		std::vector<FirstOrderAssignment> result;
		const auto enumerate = [&](this const auto& self, size_t index,
			FirstOrderAssignment assignment, size_t introduced) -> void {
			if (index == variables.size()) {
				++statistics.generated_substitutions;
				result.emplace_back(std::move(assignment));
				return;
			}
			for (const auto& object : persistent) {
				self(index + 1, assignment.Extended(variables[index], object), introduced);
			}
			const size_t usable = equality_types ? std::min(fresh.size(), introduced + 1) : fresh.size();
			for (size_t i = 0; i < usable; ++i) {
				self(index + 1, assignment.Extended(variables[index], fresh[i]),
					equality_types && i == introduced ? introduced + 1 : introduced);
			}
		};
		enumerate(0, source, 0);
		return result;
	}

	std::vector<BoundTransition> Instantiate(const CgTransition& transition,
		const CgState& target, const FirstOrderAssignment& source_bindings,
		const Situation& state, const BasicActionTheory& bat, ObjectSet objects,
		const ObjectSet& reserve, DomainSemantics semantics, ArenaStatistics& statistics) {
		for (const auto& action : transition.act.Actions()) {
			for (const auto& term : action.terms) {
				if (const auto* object = std::get_if<Object>(&term)) objects.emplace(*object);
			}
		}
		std::vector<BoundTransition> result;
		for (const auto& assignment : EnumerateAssignments(transition.vars, source_bindings,
			objects, reserve, semantics == DomainSemantics::InfiniteGeneric, statistics)) {
			if (!Holds(transition.condition, state, bat, assignment, objects, semantics)) {
				++statistics.rejected_guards;
				continue;
			}
			result.push_back({Ground(transition.act, assignment),
				assignment.Project(target.live_variables)});
		}
		return result;
	}

	Variable ScopedVariable(size_t component, const Variable& variable) {
		return Variable{"@r" + std::to_string(component) + "-" + variable.name()};
	}

	std::vector<BoundFacilityTransition> InstantiateFacility(
		const nightly::Transition<TopologyState, TopologyTransition>& transition,
		const ArenaState& source, const Situation& state, const BasicActionTheory& bat,
		ObjectSet objects, const ObjectSet& reserve, DomainSemantics semantics,
		const std::vector<Resource>& resources, ArenaStatistics& statistics) {
		const auto& components = transition.label().components;
		if (components.size() != source.facility_bindings.size()
			|| components.size() != transition.to().size()) {
			throw std::invalid_argument("Facility product lost a resource component");
		}

		std::vector<CgTransition> scoped;
		std::vector<VariableRenaming> renamings;
		FirstOrderAssignment combined_source;
		std::vector<Variable> all_variables;
		for (size_t i = 0; i < components.size(); ++i) {
			VariableRenaming renaming;
			for (const auto& [variable, value] : source.facility_bindings[i].Values()) {
				const Variable scoped_variable = ScopedVariable(i, variable);
				renaming.emplace(variable, scoped_variable);
				if (const auto* object = std::get_if<Object>(&value)) combined_source.Set(scoped_variable, *object);
			}
			for (const auto& variable : components[i].label().vars) {
				renaming.try_emplace(variable, ScopedVariable(i, variable));
				all_variables.emplace_back(renaming.at(variable));
			}
			CgTransition copy = components[i].label();
			copy.act = RenameVariables(copy.act, renaming);
			copy.condition = RenameFreeVariables(copy.condition, renaming);
			copy.vars.clear();
			scoped.emplace_back(std::move(copy));
			renamings.emplace_back(std::move(renaming));
		}

		for (const auto& component : scoped) {
			for (const auto& action : component.act.Actions()) {
				for (const auto& term : action.terms) {
					if (const auto* object = std::get_if<Object>(&term)) objects.emplace(*object);
				}
			}
		}

		std::vector<BoundFacilityTransition> result;
		for (const auto& assignment : EnumerateAssignments(all_variables, combined_source,
			objects, reserve, semantics == DomainSemantics::InfiniteGeneric, statistics)) {
			bool guards = true;
			for (const auto& component : scoped) {
				if (!Holds(component.condition, state, bat, assignment, objects, semantics)) {
					guards = false;
					break;
				}
			}
			if (!guards) {
				++statistics.rejected_guards;
				continue;
			}

			BoundFacilityTransition bound;
			bound.target_bindings.resize(components.size());
			for (size_t i = 0; i < components.size(); ++i) {
				const ResourceIndex index = i < resources.size() ? resources[i].index : i;
				bound.action.steps.push_back({index, Ground(scoped[i].act, assignment)});
				for (const auto& original : transition.to()[i].live_variables) {
					const auto found = renamings[i].find(original);
					if (found != renamings[i].end() && assignment.Contains(found->second)) {
						bound.target_bindings[i].Set(original, assignment.GetObject(found->second));
					}
				}
			}
			result.emplace_back(std::move(bound));
		}
		return result;
	}

	ObjectRenaming IdentityWitness(const ObjectSet& support) {
		ObjectRenaming result;
		for (const auto& object : Ordered(support)) {
			if (object.IsIdentifier()) result.emplace_back(object, object);
		}
		return result;
	}

	bool SameControl(const ArenaState& lhs, const ArenaState& rhs) {
		return lhs.owner == rhs.owner && lhs.recipe_control == rhs.recipe_control
			&& lhs.facility_control == rhs.facility_control;
	}

	std::optional<ObjectRenaming> FindTargetMatch(const ArenaState& source,
		const ArenaState& candidate, const ArenaState& representative,
		ArenaStatistics& statistics) {
		++statistics.isomorphism_checks;
		if (!SameControl(candidate, representative)) return std::nullopt;
		const ObjectSet source_support = StateSupport(source);
		const ObjectSet candidate_support = StateSupport(candidate);
		const ObjectSet representative_support = StateSupport(representative);

		std::vector<Object> fresh_candidate;
		std::vector<Object> fresh_representative;
		for (const auto& object : Ordered(candidate_support)) {
			if (!source_support.contains(object)) fresh_candidate.emplace_back(object);
		}
		for (const auto& object : Ordered(representative_support)) {
			if (!source_support.contains(object)) fresh_representative.emplace_back(object);
		}
		if (fresh_candidate.size() != fresh_representative.size()) return std::nullopt;

		ObjectRenaming fixed = IdentityWitness(source_support);
		std::ranges::sort(fresh_representative, ObjectLess);
		do {
			ObjectRenaming mapping = fixed;
			for (size_t i = 0; i < fresh_candidate.size(); ++i) {
				mapping.emplace_back(fresh_candidate[i], fresh_representative[i]);
			}
			if (RenameState(candidate, mapping) == representative) return mapping;
		} while (std::ranges::next_permutation(fresh_representative, ObjectLess).found);
		return std::nullopt;
	}

	void CompleteEdgeWitness(ObjectRenaming& mapping, const ObjectSet& edge_support,
		const ObjectSet& pool) {
		ObjectSet used;
		for (const auto& [from, to] : mapping) {
			(void)from;
			used.emplace(to);
		}
		for (const auto& object : Ordered(edge_support)) {
			if (object.IsRigid() || std::ranges::any_of(mapping,
				[&](const auto& pair) { return pair.first == object; })) continue;
			Object target = object;
			if (!pool.contains(target) || used.contains(target)) {
				const auto ordered_pool = Ordered(pool);
				const auto available = std::ranges::find_if(ordered_pool, [&](const Object& value) {
					return !used.contains(value);
				});
				if (available == ordered_pool.end()) {
					throw std::invalid_argument("Abstraction pool cannot represent an edge support");
				}
				target = *available;
			}
			mapping.emplace_back(object, target);
			used.emplace(target);
		}
		std::ranges::sort(mapping, [](const auto& lhs, const auto& rhs) {
			return ObjectLess(lhs.first, rhs.first);
		});
	}

	bool Cancelled(const SynthesisOptions& options) {
		return options.deadline && std::chrono::steady_clock::now() >= *options.deadline;
	}

	bool ContainsRenameableConstant(const Action& action) {
		return std::ranges::any_of(action.terms, [](const Term& term) {
			const auto* object = std::get_if<Object>(&term);
			return object != nullptr && object->IsIdentifier();
		});
	}

	bool ContainsRenameableConstant(const CompoundAction& action) {
		return std::ranges::any_of(action.Actions(), [](const Action& component) {
			return ContainsRenameableConstant(component);
		});
	}

	bool ContainsRenameableConstant(const Formula& formula) {
		if (const auto* object = std::get_if<Object>(&formula)) return object->IsIdentifier();
		if (const auto* predicate = std::get_if<Predicate>(&formula)) {
			return std::ranges::any_of(predicate->terms(), [](const Term& term) {
				const auto* object = std::get_if<Object>(&term);
				return object != nullptr && object->IsIdentifier();
			});
		}
		if (const auto* action = std::get_if<Action>(&formula)) {
			return ContainsRenameableConstant(*action);
		}
		if (const auto* action = std::get_if<CompoundAction>(&formula)) {
			return ContainsRenameableConstant(*action);
		}
		if (const auto* unary = std::get_if<Box<UnaryConnective>>(&formula)) {
			return ContainsRenameableConstant((*unary)->child());
		}
		if (const auto* binary = std::get_if<Box<BinaryConnective>>(&formula)) {
			return ContainsRenameableConstant((*binary)->lhs())
				|| ContainsRenameableConstant((*binary)->rhs());
		}
		if (const auto* quantifier = std::get_if<Box<Quantifier>>(&formula)) {
			return ContainsRenameableConstant((*quantifier)->child());
		}
		return false;
	}

	bool ContainsRenameableConstant(const BasicActionTheory& bat) {
		if (std::ranges::any_of(bat.objects, [](const Object& object) {
			return object.IsIdentifier();
		})) return true;
		for (const auto& [name, poss] : bat.pre) {
			(void)name;
			if (std::ranges::any_of(poss.Terms(), [](const Term& term) {
				const auto* object = std::get_if<Object>(&term);
				return object != nullptr && object->IsIdentifier();
			}) || ContainsRenameableConstant(poss.Form())) return true;
		}
		for (const auto& [name, successor] : bat.successors) {
			(void)name;
			if (std::ranges::any_of(successor.Terms(), [](const Term& term) {
				const auto* object = std::get_if<Object>(&term);
				return object != nullptr && object->IsIdentifier();
			}) || ContainsRenameableConstant(successor.Form())) return true;
		}
		return false;
	}

	bool ContainsRenameableConstant(const CharacteristicGraph& graph) {
		for (const auto& [state, outgoing] : graph.lts.states()) {
			if (ContainsRenameableConstant(state.final_condition)) return true;
			for (const auto& edge : outgoing.transitions()) {
				if (ContainsRenameableConstant(edge.label().act)
					|| ContainsRenameableConstant(edge.label().condition)) return true;
			}
		}
		return false;
	}

	bool ContainsUndeclaredRigidConstant(const Action& action, const ObjectSet& declared) {
		return std::ranges::any_of(action.terms, [&](const Term& term) {
			const auto* object = std::get_if<Object>(&term);
			return object != nullptr && object->IsRigid() && !declared.contains(*object);
		});
	}

	bool ContainsUndeclaredRigidConstant(const CompoundAction& action, const ObjectSet& declared) {
		return std::ranges::any_of(action.Actions(), [&](const Action& component) {
			return ContainsUndeclaredRigidConstant(component, declared);
		});
	}

	bool ContainsUndeclaredRigidConstant(const Formula& formula, const ObjectSet& declared) {
		if (const auto* object = std::get_if<Object>(&formula)) {
			return object->IsRigid() && !declared.contains(*object);
		}
		if (const auto* predicate = std::get_if<Predicate>(&formula)) {
			return std::ranges::any_of(predicate->terms(), [&](const Term& term) {
				const auto* object = std::get_if<Object>(&term);
				return object != nullptr && object->IsRigid() && !declared.contains(*object);
			});
		}
		if (const auto* action = std::get_if<Action>(&formula)) {
			return ContainsUndeclaredRigidConstant(*action, declared);
		}
		if (const auto* action = std::get_if<CompoundAction>(&formula)) {
			return ContainsUndeclaredRigidConstant(*action, declared);
		}
		if (const auto* unary = std::get_if<Box<UnaryConnective>>(&formula)) {
			return ContainsUndeclaredRigidConstant((*unary)->child(), declared);
		}
		if (const auto* binary = std::get_if<Box<BinaryConnective>>(&formula)) {
			return ContainsUndeclaredRigidConstant((*binary)->lhs(), declared)
				|| ContainsUndeclaredRigidConstant((*binary)->rhs(), declared);
		}
		if (const auto* quantifier = std::get_if<Box<Quantifier>>(&formula)) {
			return ContainsUndeclaredRigidConstant((*quantifier)->child(), declared);
		}
		return false;
	}

	bool ContainsUndeclaredRigidConstant(const BasicActionTheory& bat, const ObjectSet& declared) {
		for (const auto& [name, fluent] : bat.Initial().Fluents()) {
			(void)name;
			for (const auto& tuple : fluent.TrueTuples()) {
				if (std::ranges::any_of(tuple, [&](const Object& object) {
					return object.IsRigid() && !declared.contains(object);
				})) return true;
			}
		}
		for (const auto& [name, poss] : bat.pre) {
			(void)name;
			if (std::ranges::any_of(poss.Terms(), [&](const Term& term) {
				const auto* object = std::get_if<Object>(&term);
				return object != nullptr && object->IsRigid() && !declared.contains(*object);
			}) || ContainsUndeclaredRigidConstant(poss.Form(), declared)) return true;
		}
		for (const auto& [name, successor] : bat.successors) {
			(void)name;
			if (std::ranges::any_of(successor.Terms(), [&](const Term& term) {
				const auto* object = std::get_if<Object>(&term);
				return object != nullptr && object->IsRigid() && !declared.contains(*object);
			}) || ContainsUndeclaredRigidConstant(successor.Form(), declared)) return true;
		}
		return false;
	}

	bool ContainsUndeclaredRigidConstant(const CharacteristicGraph& graph,
		const ObjectSet& declared) {
		for (const auto& [state, outgoing] : graph.lts.states()) {
			if (ContainsUndeclaredRigidConstant(state.final_condition, declared)) return true;
			for (const auto& edge : outgoing.transitions()) {
				if (ContainsUndeclaredRigidConstant(edge.label().act, declared)
					|| ContainsUndeclaredRigidConstant(edge.label().condition, declared)) return true;
			}
		}
		return false;
	}

	template <typename Lts>
	size_t TransitionCount(const Lts& lts) {
		size_t result = 0;
		for (const auto& [state, outgoing] : lts.states()) {
			(void)state;
			result += outgoing.transitions().size();
		}
		return result;
	}

	class Builder {
	public:
		Builder(const SynthesisProblem& problem, const SynthesisOptions& options)
			: problem_(problem), options_(options),
			recipe_(problem.recipe, ProgramType::Recipe) {
			for (const auto& resource : problem.facility.resources) {
				resources_.emplace_back(resource.program, ProgramType::Resource);
			}
			topology_ = std::make_unique<IncrementalTopology>(&resources_);
			arena_.statistics.recipe_graph_states = recipe_.lts.states().size();
			arena_.statistics.recipe_graph_edges = TransitionCount(recipe_.lts);
			for (const auto& graph : resources_) {
				arena_.statistics.resource_graph_states += graph.lts.states().size();
				arena_.statistics.resource_graph_edges += TransitionCount(graph.lts);
			}
		}

		ArenaBuildResult Run() {
			ArenaBuildResult result;
			const auto validation = ValidateFacility(problem_.facility);
			if (!problem_.recipe) result.diagnostics.emplace_back("Recipe program is missing");
			result.diagnostics.insert(result.diagnostics.end(), validation.begin(), validation.end());
			if (!result.diagnostics.empty()) return result;

			const ObjectSet& declared_rigid = problem_.facility.bat.rigid_objects;
			if (ContainsUndeclaredRigidConstant(problem_.facility.bat, declared_rigid)
				|| ContainsUndeclaredRigidConstant(recipe_, declared_rigid)
				|| std::ranges::any_of(resources_, [&](const CharacteristicGraph& graph) {
					return ContainsUndeclaredRigidConstant(graph, declared_rigid);
				})) {
				throw std::invalid_argument(
					"Static model syntax contains an undeclared rigid constant");
			}

			infinite_ = std::holds_alternative<FaithfulAbstractionBackend>(options_.backend);
			if (infinite_) {
				if (ContainsRenameableConstant(problem_.facility.bat)
					|| std::ranges::any_of(problem_.facility.resources, [](const Resource& resource) {
						return ContainsRenameableConstant(resource.bat);
					}) || ContainsRenameableConstant(recipe_)
					|| std::ranges::any_of(resources_, [](const CharacteristicGraph& graph) {
						return ContainsRenameableConstant(graph);
					})) {
					throw std::invalid_argument(
						"Faithful abstraction forbids renameable identifier constants in static model syntax");
				}
				const auto& backend = std::get<FaithfulAbstractionBackend>(options_.backend);
				order_ = backend.worklist_order;
				arena_.bounds = ComputeArenaBounds(problem_, recipe_, resources_, backend.active_domain_bound);
				InitializePool();
			} else {
				carrier_ = std::get<FiniteDomainBackend>(options_.backend).explicit_objects;
				carrier_.insert(problem_.facility.bat.rigid_objects.begin(),
					problem_.facility.bat.rigid_objects.end());
			}

			ArenaState initial;
			initial.owner = ArenaOwner::Environment;
			initial.recipe_control = recipe_.lts.initial_state();
			initial.facility_control = topology_->initial_state();
			initial.facility_bindings.resize(resources_.size());
			initial.interpretation = problem_.facility.bat.Initial();
			arena_.initial = arena_.AddState(std::move(initial));

			ArenaState goal;
			goal.owner = ArenaOwner::Goal;
			arena_.goal = arena_.AddState(std::move(goal));
			ArenaState lose;
			lose.owner = ArenaOwner::Lose;
			arena_.lose = arena_.AddState(std::move(lose));
			arena_.AddEdge({arena_.goal, arena_.goal, AuxiliaryLabel::Sink, 0, {}});
			arena_.AddEdge({arena_.lose, arena_.lose, AuxiliaryLabel::Sink, 0, {}});

			pending_.push_back(arena_.initial);
			discovery_cost_.resize(arena_.states.size(), 0);
			while (!pending_.empty()) {
				if (Cancelled(options_)) {
					CaptureProductStatistics();
					result.status = ArenaBuildStatus::Cancelled;
					result.arena = std::move(arena_);
					return result;
				}
				const ArenaStateId state = PopPending();
				Expand(state);
			}
			AddDeadEdges();
			CaptureProductStatistics();
			result.status = ArenaBuildStatus::Complete;
			result.arena = std::move(arena_);
			return result;
		}

	private:
		const SynthesisProblem& problem_;
		const SynthesisOptions& options_;
		CharacteristicGraph recipe_;
		std::vector<CharacteristicGraph> resources_;
		std::unique_ptr<IncrementalTopology> topology_;
		Arena arena_;
		bool infinite_ = false;
		WorklistOrder order_ = WorklistOrder::BreadthFirst;
		ObjectSet carrier_;
		ObjectSet pool_;
		std::vector<ArenaStateId> pending_;
		std::vector<uint64_t> discovery_cost_;
		size_t candidate_filter_samples_ = 0;
		size_t possible_samples_ = 0;
		size_t observation_samples_ = 0;
		size_t cost_samples_ = 0;
		size_t equivariance_samples_ = 0;
		static constexpr size_t callback_sample_limit_ = 8;

		void CaptureProductStatistics() {
			arena_.statistics.product_states = topology_->lts().states().size();
			arena_.statistics.product_edges = TransitionCount(topology_->lts());
		}

		bool TakeCallbackSample(size_t& counter) {
			if (counter >= callback_sample_limit_) return false;
			++counter;
			++arena_.statistics.callback_contract_samples;
			return true;
		}

		void InitializePool() {
			const ObjectSet initial_support = ActiveIdentifiers(problem_.facility.bat.Initial());
			if (initial_support.size() > arena_.bounds.active_domain) {
				throw std::invalid_argument("Initial support exceeds the supplied active-domain bound");
			}
			pool_ = initial_support;
			for (size_t i = 0; pool_.size() < arena_.bounds.pool; ++i) {
				pool_.emplace(Object::Identifier("@scs-id-" + std::to_string(i)));
			}
		}

		ArenaStateId PopPending() {
			if (order_ == WorklistOrder::BreadthFirst) {
				const ArenaStateId result = pending_.front();
				pending_.erase(pending_.begin());
				return result;
			}
			const auto score = [&](ArenaStateId id) {
				const uint64_t owner_bias = order_ == WorklistOrder::Greedy
					&& arena_.states[id].owner == ArenaOwner::Controller ? 0 : 1;
				return std::pair{owner_bias, discovery_cost_[id]};
			};
			const auto best = std::ranges::min_element(pending_, {}, score);
			const ArenaStateId result = *best;
			pending_.erase(best);
			return result;
		}

		std::pair<ObjectSet, ObjectSet> ExpansionCarrier(const ArenaState& state) const {
			if (!infinite_) return {carrier_, {}};
			ObjectSet objects = StateSupport(state);
			objects.insert(problem_.facility.bat.rigid_objects.begin(),
				problem_.facility.bat.rigid_objects.end());
			ObjectSet reserve;
			for (const auto& object : Ordered(pool_)) {
				if (!objects.contains(object)) {
					objects.emplace(object);
					reserve.emplace(object);
					if (reserve.size() == arena_.bounds.fresh_edge) break;
				}
			}
			if (reserve.size() != arena_.bounds.fresh_edge) {
				throw std::invalid_argument("Abstraction pool has insufficient fresh reserve");
			}
			return {objects, reserve};
		}

		void Expand(ArenaStateId id) {
			if (arena_.states[id].owner == ArenaOwner::Environment) ExpandEnvironment(id);
			else if (arena_.states[id].owner == ArenaOwner::Controller) ExpandController(id);
		}

		void ExpandEnvironment(ArenaStateId id) {
			const ArenaState source = arena_.states[id];
			const auto [objects, reserve] = ExpansionCarrier(source);
			const DomainSemantics semantics = infinite_ ? DomainSemantics::InfiniteGeneric : DomainSemantics::Finite;
			const auto& control_state = recipe_.lts.at(source.recipe_control);
			for (const auto& transition : control_state.transitions()) {
				for (const auto& bound : Instantiate(transition.label(), transition.to(),
					source.recipe_bindings, source.interpretation, problem_.facility.bat,
					objects, reserve, semantics, arena_.statistics)) {
					ArenaState target = source;
					target.owner = ArenaOwner::Controller;
					target.recipe_control = transition.to();
					target.recipe_bindings = bound.target_bindings;
					target.pending = bound.action;
					StoreCandidate(id, ArenaLabel{bound.action}, std::move(target), 0);
				}
			}
			if (Holds(source.recipe_control.final_condition, source.interpretation,
				problem_.facility.bat, source.recipe_bindings, objects, semantics)) {
				arena_.AddEdge({id, arena_.goal, AuxiliaryLabel::Stop, 0,
					IdentityWitness(StateSupport(source))});
			}
		}

		void ExpandController(ArenaStateId id) {
			const ArenaState source = arena_.states[id];
			const auto [objects, reserve] = ExpansionCarrier(source);
			const DomainSemantics semantics = infinite_ ? DomainSemantics::InfiniteGeneric : DomainSemantics::Finite;
			const auto topology_transition_state = topology_->at(source.facility_control);
			for (const auto& transition : topology_transition_state.transitions()) {
				if (problem_.facility.callbacks.may_be_possible) {
					JointAction schema;
					for (size_t i = 0; i < transition.label().components.size(); ++i) {
						const ResourceIndex index = i < problem_.facility.resources.size()
							? problem_.facility.resources[i].index : i;
						schema.steps.push_back({index, transition.label().components[i].label().act});
					}
					const bool accepted = problem_.facility.callbacks.may_be_possible(schema);
					if (TakeCallbackSample(candidate_filter_samples_)
						&& accepted != problem_.facility.callbacks.may_be_possible(schema)) {
						throw std::invalid_argument("Facility candidate filter is not deterministic");
					}
					if (!accepted) {
						++arena_.statistics.rejected_preconditions;
						continue;
					}
				}
				for (auto& bound : InstantiateFacility(transition, source, source.interpretation,
					problem_.facility.bat, objects, reserve, semantics,
					problem_.facility.resources, arena_.statistics)) {
					if (!SamplePossibleContract(source, bound.action, objects, semantics)) {
						++arena_.statistics.rejected_preconditions;
						continue;
					}
					const auto observation = problem_.facility.Observe(bound.action);
					if (TakeCallbackSample(observation_samples_)
						&& observation != problem_.facility.Observe(bound.action)) {
						throw std::invalid_argument("Facility observation callback is not deterministic");
					}
					if (observation && observation != source.pending) {
						++arena_.statistics.rejected_observations;
						continue;
					}

					Situation progressed = infinite_
						? source.interpretation.Do(bound.action.IndexedFlatten(), problem_.facility.bat, true)
						: source.interpretation.Do(bound.action.IndexedFlatten(), problem_.facility.bat, objects, true);
					++arena_.statistics.progression_calls;
					if (infinite_ && RenameableActiveDomainSize(progressed) > arena_.bounds.active_domain) {
						throw std::invalid_argument("Progression exceeds the supplied active-domain bound");
					}

					ArenaState target = source;
					target.facility_control = transition.to();
					target.facility_bindings = std::move(bound.target_bindings);
					target.interpretation = std::move(progressed);
					if (observation) {
						target.owner = ArenaOwner::Environment;
						target.pending.reset();
					}

					const FacilityProgramStateView source_view{&source.facility_control,
						&source.facility_bindings};
					const FacilityProgramStateView target_view{&target.facility_control,
						&target.facility_bindings};
					const uint64_t cost = problem_.facility.Cost(source_view, source.interpretation,
						bound.action, target_view, target.interpretation);
					if (TakeCallbackSample(cost_samples_)
						&& cost != problem_.facility.Cost(source_view, source.interpretation,
						bound.action, target_view, target.interpretation)) {
						throw std::invalid_argument("Facility cost callback is not deterministic");
					}
					if (infinite_ && TakeCallbackSample(equivariance_samples_)) {
						SampleEquivariance(source, bound.action, target, observation, cost, semantics);
					}
					StoreCandidate(id, ArenaLabel{std::move(bound.action)}, std::move(target), cost);
				}
			}
		}

		ObjectRenaming SampleRenaming(const ArenaState& source, const JointAction& action,
			const ArenaState& target) const {
			const ObjectSet support = EdgeSupport(source, ArenaLabel{action}, target);
			ObjectSet used = support;
			ObjectRenaming renaming;
			size_t counter = 0;
			for (const auto& object : Ordered(support)) {
				Object renamed;
				do {
					renamed = Object::Identifier("@scs-equiv-" + std::to_string(counter++));
				} while (used.contains(renamed));
				used.emplace(renamed);
				renaming.emplace_back(object, std::move(renamed));
			}
			return renaming;
		}

		ObjectSet RenameObjects(const ObjectSet& objects, const ObjectRenaming& renaming) const {
			ObjectSet result;
			for (const auto& object : objects) result.emplace(RenameObject(object, renaming));
			return result;
		}

		bool SamplePossibleContract(const ArenaState& source, const JointAction& action,
			const ObjectSet& objects, DomainSemantics semantics) {
			const bool result = problem_.facility.Possible(action, source.interpretation, objects, semantics);
			if (!TakeCallbackSample(possible_samples_)) return result;
			if (result != problem_.facility.Possible(action, source.interpretation, objects, semantics)) {
				throw std::invalid_argument("Facility executability callback is not deterministic");
			}
			if (!infinite_) return result;
			const ObjectRenaming renaming = SampleRenaming(source, action, source);
			const ArenaState renamed_source = RenameState(source, renaming);
			const auto renamed_action = std::get<JointAction>(RenameLabel(ArenaLabel{action}, renaming));
			if (result != problem_.facility.Possible(renamed_action, renamed_source.interpretation,
				RenameObjects(objects, renaming), semantics)) {
				throw std::invalid_argument("Facility executability callback violates sampled equivariance");
			}
			return result;
		}

		void SampleEquivariance(const ArenaState& source, const JointAction& action,
			const ArenaState& target, const std::optional<CompoundAction>& observation,
			uint64_t cost, DomainSemantics semantics) {
			const ObjectRenaming renaming = SampleRenaming(source, action, target);
			const ArenaState renamed_source = RenameState(source, renaming);
			const ArenaState renamed_target = RenameState(target, renaming);
			const auto renamed_action = std::get<JointAction>(RenameLabel(ArenaLabel{action}, renaming));
			const auto renamed_observation = problem_.facility.Observe(renamed_action);
			const auto expected_observation = observation
				? std::optional<CompoundAction>{std::get<CompoundAction>(
					RenameLabel(ArenaLabel{*observation}, renaming))}
				: std::nullopt;
			if (renamed_observation != expected_observation) {
				throw std::invalid_argument("Facility observation callback violates sampled equivariance");
			}
			const ObjectSet renamed_objects = RelevantObjects(renamed_source.interpretation,
				problem_.facility.bat, renamed_action.Flatten());
			if (!problem_.facility.Possible(renamed_action, renamed_source.interpretation,
				renamed_objects, semantics)) {
				throw std::invalid_argument("Facility executability callback violates sampled equivariance");
			}
			const FacilityProgramStateView renamed_source_view{&renamed_source.facility_control,
				&renamed_source.facility_bindings};
			const FacilityProgramStateView renamed_target_view{&renamed_target.facility_control,
				&renamed_target.facility_bindings};
			if (cost != problem_.facility.Cost(renamed_source_view, renamed_source.interpretation,
				renamed_action, renamed_target_view, renamed_target.interpretation)) {
				throw std::invalid_argument("Facility cost callback violates sampled equivariance");
			}
		}

		void StoreCandidate(ArenaStateId source_id, ArenaLabel label,
			ArenaState candidate, uint64_t cost) {
			// Adding a new target may reallocate arena_.states. Keep the source
			// value stable while constructing the edge witness.
			const ArenaState source = arena_.states[source_id];
			ArenaStateId target_id = std::numeric_limits<ArenaStateId>::max();
			ObjectRenaming witness;
			if (infinite_) {
				for (ArenaStateId existing = 0; existing < arena_.states.size(); ++existing) {
					if (arena_.states[existing].owner == ArenaOwner::Goal
						|| arena_.states[existing].owner == ArenaOwner::Lose) continue;
					if (auto match = FindTargetMatch(source, candidate, arena_.states[existing],
						arena_.statistics)) {
						target_id = existing;
						witness = std::move(*match);
						++arena_.statistics.isomorphism_matches;
						break;
					}
				}
			} else {
				const auto existing = std::ranges::find(arena_.states, candidate);
				if (existing != arena_.states.end()) target_id = std::distance(arena_.states.begin(), existing);
			}

			if (target_id == std::numeric_limits<ArenaStateId>::max()) {
				target_id = arena_.AddState(std::move(candidate));
				pending_.push_back(target_id);
				discovery_cost_.resize(arena_.states.size(), std::numeric_limits<uint64_t>::max());
				discovery_cost_[target_id] = cost;
				witness = IdentityWitness(EdgeSupport(source, label, arena_.states[target_id]));
			} else if (infinite_) {
				CompleteEdgeWitness(witness, EdgeSupport(source, label, candidate), pool_);
				label = RenameLabel(label, witness);
			} else {
				witness = IdentityWitness(EdgeSupport(source, label, arena_.states[target_id]));
			}

			if (target_id == source_id) {
				++arena_.statistics.removed_self_loops;
				return;
			}

			if (source.owner == ArenaOwner::Controller) {
				const auto visible = std::get_if<JointAction>(&label)
					? problem_.facility.Observe(std::get<JointAction>(label)) : std::nullopt;
				for (auto edge_id : arena_.outgoing[source_id]) {
					auto& edge = arena_.edges[edge_id];
					if (edge.target != target_id || !std::holds_alternative<JointAction>(edge.label)) continue;
					if (problem_.facility.Observe(std::get<JointAction>(edge.label)) == visible) {
						if (edge.cost <= cost) {
							++arena_.statistics.dominated_edges;
							return;
						}
						edge = {source_id, target_id, std::move(label), cost, std::move(witness)};
						++arena_.statistics.dominated_edges;
						return;
					}
				}
			}
			arena_.AddEdge({source_id, target_id, std::move(label), cost, std::move(witness)});
		}

		void AddDeadEdges() {
			for (ArenaStateId id = 0; id < arena_.states.size(); ++id) {
				const auto owner = arena_.states[id].owner;
				if ((owner == ArenaOwner::Environment || owner == ArenaOwner::Controller)
					&& arena_.outgoing[id].empty()) {
					arena_.AddEdge({id, arena_.lose, AuxiliaryLabel::Dead, 0,
						IdentityWitness(StateSupport(arena_.states[id]))});
				}
			}
		}
	};

}

	ArenaStateId Arena::AddState(ArenaState state) {
		const ArenaStateId id = states.size();
		states.emplace_back(std::move(state));
		outgoing.emplace_back();
		return id;
	}

	ArenaEdgeId Arena::AddEdge(ArenaEdge edge) {
		const ArenaEdgeId id = edges.size();
		outgoing.at(edge.source).push_back(id);
		edges.emplace_back(std::move(edge));
		return id;
	}

	ArenaBounds ComputeArenaBounds(const SynthesisProblem&, const CharacteristicGraph& recipe,
		const std::vector<CharacteristicGraph>& resources, size_t active_domain_bound) {
		ArenaBounds bounds;
		bounds.active_domain = active_domain_bound;
		for (const auto& [state, outgoing] : recipe.lts.states()) {
			(void)outgoing;
			bounds.recipe_live = std::max(bounds.recipe_live, state.live_variables.size());
		}
		for (const auto& [state, outgoing] : recipe.lts.states()) {
			(void)state;
			for (const auto& edge : outgoing.transitions()) {
				bounds.recipe_action_arity = std::max(bounds.recipe_action_arity,
					edge.label().act.Actions().size() == 0 ? size_t{0} :
						std::accumulate(edge.label().act.Actions().begin(), edge.label().act.Actions().end(),
							size_t{0}, [](size_t n, const Action& action) { return n + action.terms.size(); }));
				bounds.fresh_edge = std::max(bounds.fresh_edge, edge.label().vars.size());
			}
		}
		size_t facility_fresh = 0;
		for (const auto& graph : resources) {
			size_t live = 0;
			size_t fresh = 0;
			for (const auto& [state, outgoing] : graph.lts.states()) {
				live = std::max(live, state.live_variables.size());
				for (const auto& edge : outgoing.transitions()) fresh = std::max(fresh, edge.label().vars.size());
			}
			bounds.facility_live += live;
			facility_fresh += fresh;
		}
		bounds.fresh_edge = std::max(bounds.fresh_edge, facility_fresh);
		bounds.support = bounds.active_domain + bounds.recipe_live
			+ bounds.facility_live + bounds.recipe_action_arity;
		bounds.pool = 2 * bounds.support + bounds.fresh_edge;
		return bounds;
	}

	ObjectSet StateSupport(const ArenaState& state) {
		ObjectSet result = ActiveIdentifiers(state.interpretation);
		AddAssignmentObjects(result, state.recipe_bindings);
		for (const auto& assignment : state.facility_bindings) AddAssignmentObjects(result, assignment);
		if (state.pending) AddActionObjects(result, *state.pending);
		for (auto it = result.begin(); it != result.end();) {
			if (it->IsRigid()) it = result.erase(it);
			else ++it;
		}
		return result;
	}

	ObjectSet EdgeSupport(const ArenaState& source, const ArenaLabel& label,
		const ArenaState& target) {
		ObjectSet result = StateSupport(source);
		const auto target_support = StateSupport(target);
		result.insert(target_support.begin(), target_support.end());
		AddLabelObjects(result, label);
		for (auto it = result.begin(); it != result.end();) {
			if (it->IsRigid()) it = result.erase(it);
			else ++it;
		}
		return result;
	}

	ArenaState RenameState(const ArenaState& state, const ObjectRenaming& renaming) {
		ArenaState result = state;
		result.recipe_bindings = RenameAssignment(state.recipe_bindings, renaming);
		for (size_t i = 0; i < state.facility_bindings.size(); ++i) {
			result.facility_bindings[i] = RenameAssignment(state.facility_bindings[i], renaming);
		}
		result.interpretation = RenameInterpretation(state.interpretation, renaming);
		if (state.pending) {
			result.pending = std::get<CompoundAction>(RenameLabel(ArenaLabel{*state.pending}, renaming));
		}
		return result;
	}

	ArenaLabel RenameLabel(const ArenaLabel& label, const ObjectRenaming& renaming) {
		const auto rename_compound = [&](const CompoundAction& action) {
			CompoundAction result;
			for (const auto& component : action.Actions()) {
				Action renamed = component;
				for (auto& term : renamed.terms) {
					if (auto* object = std::get_if<Object>(&term)) *object = RenameObject(*object, renaming);
				}
				result.AppendAction(std::move(renamed));
			}
			return result;
		};
		if (const auto* request = std::get_if<CompoundAction>(&label)) return rename_compound(*request);
		if (const auto* joint = std::get_if<JointAction>(&label)) {
			JointAction result = *joint;
			for (auto& step : result.steps) step.action = rename_compound(step.action);
			return result;
		}
		return std::get<AuxiliaryLabel>(label);
	}

	bool IsBijectionWitness(const ObjectRenaming& witness) {
		ObjectSet from;
		ObjectSet to;
		for (const auto& [source, target] : witness) {
			if (!source.IsIdentifier() || !target.IsIdentifier()
				|| !from.emplace(source).second || !to.emplace(target).second) return false;
		}
		return true;
	}

	ArenaBuildResult BuildArena(const SynthesisProblem& problem,
		const SynthesisOptions& options) {
		try {
			return Builder{problem, options}.Run();
		} catch (const std::exception& error) {
			ArenaBuildResult result;
			result.status = ArenaBuildStatus::InvalidModel;
			result.diagnostics.emplace_back(error.what());
			return result;
		}
	}

}

size_t std::hash<scs::ArenaState>::operator()(const scs::ArenaState& state) const {
	size_t seed = static_cast<size_t>(state.owner);
	boost::hash_combine(seed, state.recipe_control.n);
	boost::hash_combine(seed, std::hash<scs::FirstOrderAssignment>{}(state.recipe_bindings));
	boost::hash_combine(seed, boost::hash_range(state.facility_control.begin(), state.facility_control.end()));
	for (const auto& assignment : state.facility_bindings) {
		boost::hash_combine(seed, std::hash<scs::FirstOrderAssignment>{}(assignment));
	}
	boost::hash_combine(seed, std::hash<scs::Situation>{}(state.interpretation));
	if (state.pending) boost::hash_combine(seed, std::hash<scs::CompoundAction>{}(*state.pending));
	return seed;
}
