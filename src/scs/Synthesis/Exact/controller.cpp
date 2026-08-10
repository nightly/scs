#include "scs/Synthesis/Exact/controller.h"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string_view>

namespace scs {
namespace {

	const Object* Lookup(const ObjectRenaming& mapping, const Object& source) {
		const auto found = std::ranges::find(mapping, source,
			[](const auto& pair) { return pair.first; });
		return found == mapping.end() ? nullptr : &found->second;
	}

	bool TargetUsed(const ObjectRenaming& mapping, const Object& target,
		const Object* except_source = nullptr) {
		return std::ranges::any_of(mapping, [&](const auto& pair) {
			return pair.second == target && (except_source == nullptr || pair.first != *except_source);
		});
	}

	bool ControllerObjectLess(const Object& lhs, const Object& rhs) {
		if (lhs.kind() != rhs.kind()) return lhs.kind() < rhs.kind();
		return lhs.name() < rhs.name();
	}

	std::string_view SourceVariableName(const Variable& variable) {
		constexpr std::string_view prefix = "__scs_pick_";
		const std::string_view name = variable.name();
		if (!name.starts_with(prefix)) return name;
		size_t separator = prefix.size();
		while (separator < name.size() && name[separator] >= '0' && name[separator] <= '9') {
			++separator;
		}
		return separator < name.size() && name[separator] == '_'
			? name.substr(separator + 1) : name;
	}

	const FirstOrderAssignment::Value* FindRecipeBinding(
		const FirstOrderAssignment& bindings, const Variable& requested) {
		if (bindings.Contains(requested)) return &bindings.Get(requested);
		const FirstOrderAssignment::Value* result = nullptr;
		for (const auto& [variable, value] : bindings.Values()) {
			if (SourceVariableName(variable) != requested.name()) continue;
			if (result != nullptr) return nullptr;
			result = &value;
		}
		return result;
	}

	std::optional<ObjectRenaming> MatchInitialInterpretation(
		const ArenaState& representative, const Interpretation& concrete) {
		const ObjectSet abstract_set = StateSupport(representative);
		std::vector<Object> abstract_identifiers(abstract_set.begin(), abstract_set.end());
		const ObjectSet concrete_set = ActiveIdentifiers(concrete);
		std::vector<Object> concrete_identifiers(concrete_set.begin(), concrete_set.end());
		if (abstract_identifiers.size() != concrete_identifiers.size()) return std::nullopt;
		std::ranges::sort(abstract_identifiers, ControllerObjectLess);
		std::ranges::sort(concrete_identifiers, ControllerObjectLess);
		do {
			ObjectRenaming mapping;
			for (size_t i = 0; i < abstract_identifiers.size(); ++i) {
				mapping.emplace_back(abstract_identifiers[i], concrete_identifiers[i]);
			}
			if (RenameState(representative, mapping).interpretation == concrete) return mapping;
		} while (std::ranges::next_permutation(concrete_identifiers, ControllerObjectLess).found);
		return std::nullopt;
	}

}

	FreshIdentifierProvider SequentialFreshIdentifiers(std::string prefix) {
		auto counter = std::make_shared<size_t>(0);
		return [prefix = std::move(prefix), counter](const ObjectSet& used) mutable {
			while (true) {
				Object candidate = Object::Identifier(prefix + std::to_string((*counter)++));
				if (!used.contains(candidate)) return candidate;
			}
		};
	}

	ControllerSession::ControllerSession(const SynthesisProblem& problem,
		const Controller& controller, FreshIdentifierProvider fresh_identifiers,
		Interpretation concrete_initial)
		: problem_(&problem), controller_(&controller),
		fresh_identifiers_(std::move(fresh_identifiers)),
		abstract_state_(controller.arena.initial),
		concrete_state_(concrete_initial.Fluents().empty()
			? problem.facility.bat.Initial() : std::move(concrete_initial)) {
		if (!fresh_identifiers_) throw std::invalid_argument("ControllerSession requires a fresh-identifier provider");
		const auto mapping = MatchInitialInterpretation(
			controller.arena.states.at(abstract_state_), concrete_state_);
		if (!mapping) throw std::invalid_argument(
			"Concrete initial interpretation is not isomorphic to the controller initial interpretation");
		representative_to_concrete_ = *mapping;
		used_identifiers_ = ActiveIdentifiers(concrete_state_);
		for (const auto& [representative, concrete] : representative_to_concrete_) {
			(void)representative;
			if (concrete.IsIdentifier()) used_identifiers_.emplace(concrete);
		}
	}

	std::optional<ObjectRenaming> ControllerSession::MatchRequest(
		const CompoundAction& representative, const CompoundAction& concrete) const {
		if (representative.Actions().size() != concrete.Actions().size()) return std::nullopt;
		ObjectRenaming mapping = representative_to_concrete_;
		for (size_t i = 0; i < representative.Actions().size(); ++i) {
			const auto& lhs = representative.Actions()[i];
			const auto& rhs = concrete.Actions()[i];
			if (lhs.name != rhs.name || lhs.terms.size() != rhs.terms.size()) return std::nullopt;
			for (size_t j = 0; j < lhs.terms.size(); ++j) {
				const auto* abstract_object = std::get_if<Object>(&lhs.terms[j]);
				const auto* concrete_object = std::get_if<Object>(&rhs.terms[j]);
				if (abstract_object == nullptr || concrete_object == nullptr) return std::nullopt;
				if (abstract_object->IsRigid()) {
					if (*abstract_object != *concrete_object) return std::nullopt;
					continue;
				}
				if (!concrete_object->IsIdentifier()) return std::nullopt;
				if (const Object* known = Lookup(mapping, *abstract_object)) {
					if (*known != *concrete_object) return std::nullopt;
				} else {
					if (TargetUsed(mapping, *concrete_object)) return std::nullopt;
					mapping.emplace_back(*abstract_object, *concrete_object);
				}
			}
		}
		return mapping;
	}

	JointAction ControllerSession::Lift(const JointAction& representative) {
		JointAction concrete = representative;
		for (auto& step : concrete.steps) {
			for (auto& action : step.action.Actions()) {
				for (auto& term : action.terms) {
					auto* object = std::get_if<Object>(&term);
					if (object == nullptr || object->IsRigid()) continue;
					const Object representative_object = *object;
					if (const Object* known = Lookup(representative_to_concrete_, representative_object)) {
						*object = *known;
					} else {
						Object fresh = fresh_identifiers_(used_identifiers_);
						if (!fresh.IsIdentifier() || used_identifiers_.contains(fresh)
							|| TargetUsed(representative_to_concrete_, fresh)) {
							throw std::runtime_error("Fresh-identifier provider returned a non-fresh identifier");
						}
						used_identifiers_.emplace(fresh);
						representative_to_concrete_.emplace_back(representative_object, fresh);
						*object = std::move(fresh);
					}
				}
			}
		}
		return concrete;
	}

	void ControllerSession::UpdateMappingForTarget(const ArenaEdge& edge, bool allocate_missing) {
		if (!IsBijectionWitness(edge.witness)) {
			throw std::runtime_error("Controller edge contains a non-bijective lifting witness");
		}
		const ObjectSet support = StateSupport(controller_->arena.states.at(edge.target));
		std::erase_if(representative_to_concrete_, [&](const auto& pair) {
			return !support.contains(pair.first);
		});
		const auto allocate = [&](const Object& representative) {
			if (!representative.IsIdentifier() || !support.contains(representative)
				|| Lookup(representative_to_concrete_, representative) != nullptr) return;
			if (!allocate_missing) {
				throw std::invalid_argument(
					"Concrete recipe-edge choice omits a live identifier binding");
			}
			Object fresh = fresh_identifiers_(used_identifiers_);
			if (!fresh.IsIdentifier() || used_identifiers_.contains(fresh)
				|| TargetUsed(representative_to_concrete_, fresh)) {
				throw std::runtime_error("Fresh-identifier provider returned a non-fresh identifier");
			}
			used_identifiers_.emplace(fresh);
			representative_to_concrete_.emplace_back(representative, std::move(fresh));
		};
		for (const auto& [candidate, representative] : edge.witness) {
			(void)candidate;
			allocate(representative);
		}
		std::vector<Object> ordered_support(support.begin(), support.end());
		std::ranges::sort(ordered_support, ControllerObjectLess);
		for (const Object& representative : ordered_support) allocate(representative);

		const auto concrete_target = RenameState(
			controller_->arena.states.at(edge.target), representative_to_concrete_);
		if (concrete_target.interpretation != concrete_state_) {
			throw std::runtime_error("Lifting witness does not reproduce the concrete target interpretation");
		}
	}

	ControllerResponse ControllerSession::Respond(const CompoundAction& concrete_request) {
		return Respond(RecipeEdgeChoice{concrete_request, {}});
	}

	ControllerResponse ControllerSession::Respond(const RecipeEdgeChoice& concrete_choice) {
		const CompoundAction& concrete_request = concrete_choice.request;
		const auto& current = controller_->arena.states.at(abstract_state_);
		if (current.owner != ArenaOwner::Environment) {
			throw std::logic_error("A recipe request is only valid at an Environment state");
		}

		std::optional<ArenaEdgeId> selected_request;
		ObjectRenaming request_mapping;
		for (const auto edge_id : controller_->arena.outgoing.at(abstract_state_)) {
			const auto& edge = controller_->arena.edges[edge_id];
			const auto* request = std::get_if<CompoundAction>(&edge.label);
			if (request == nullptr) continue;
			if (auto mapping = MatchRequest(*request, concrete_request)) {
				const auto& representative_target = controller_->arena.states.at(edge.target);
				bool bindings_match = true;
				for (const auto& [variable, concrete_value] : concrete_choice.bindings.Values()) {
					const auto* representative_value = FindRecipeBinding(
						representative_target.recipe_bindings, variable);
					if (representative_value == nullptr) {
						bindings_match = false;
						break;
					}
					const auto* representative_object = std::get_if<Object>(representative_value);
					const auto* concrete_object = std::get_if<Object>(&concrete_value);
					if (representative_object == nullptr || concrete_object == nullptr
						|| representative_object->kind() != concrete_object->kind()) {
						bindings_match = false;
						break;
					}
					if (representative_object->IsRigid()) {
						if (*representative_object != *concrete_object) bindings_match = false;
						continue;
					}
					if (const Object* known = Lookup(*mapping, *representative_object)) {
						if (*known != *concrete_object) {
							bindings_match = false;
							break;
						}
					} else if (TargetUsed(*mapping, *concrete_object)) {
						bindings_match = false;
						break;
					} else {
						mapping->emplace_back(*representative_object, *concrete_object);
					}
				}
				if (!bindings_match) continue;
				if (selected_request) {
					throw std::invalid_argument("Concrete request is ambiguous between recipe edges");
				}
				selected_request = edge_id;
				request_mapping = std::move(*mapping);
			}
		}
		if (!selected_request) throw std::invalid_argument("Concrete request is not enabled by the recipe");
		ObjectSet request_objects;
		AddGroundActionObjects(request_objects, concrete_request);
		for (const Object& object : request_objects) {
			if (object.IsIdentifier()) used_identifiers_.emplace(object);
		}
		for (const auto& [variable, value] : concrete_choice.bindings.Values()) {
			(void)variable;
			if (const auto* object = std::get_if<Object>(&value);
				object != nullptr && object->IsIdentifier()) {
				used_identifiers_.emplace(*object);
			}
		}
		representative_to_concrete_ = std::move(request_mapping);
		const auto& request_edge = controller_->arena.edges[*selected_request];
		abstract_state_ = request_edge.target;
		pending_request_ = concrete_request;
		response_budget_ = 0;
		UpdateMappingForTarget(request_edge, false);

		ControllerResponse response;
		while (controller_->arena.states.at(abstract_state_).owner == ArenaOwner::Controller) {
			const ArenaState representative_source = controller_->arena.states.at(abstract_state_);
			const ObjectRenaming source_mapping = representative_to_concrete_;
			const Interpretation concrete_source = concrete_state_;
			const auto strategy = controller_->strategy.find({abstract_state_, response_budget_});
			if (strategy == controller_->strategy.end()) {
				throw std::logic_error("Extracted controller is undefined at a reachable budget state");
			}
			const auto& edge = controller_->arena.edges.at(strategy->second);
			const auto* representative_action = std::get_if<JointAction>(&edge.label);
			if (representative_action == nullptr) throw std::logic_error("Controller selected a non-facility label");
			JointAction concrete_action = Lift(*representative_action);
			const auto objects = RelevantObjects(concrete_state_, problem_->facility.bat,
				concrete_action.Flatten());
			if (!problem_->facility.Possible(concrete_action, concrete_state_, objects,
				DomainSemantics::InfiniteGeneric)) {
				throw std::runtime_error("Lifted facility action is not executable");
			}
			const auto observation = problem_->facility.Observe(concrete_action);
			const auto target_owner = controller_->arena.states.at(edge.target).owner;
			const bool completes_response = target_owner == ArenaOwner::Environment;
			if ((completes_response && observation != pending_request_)
				|| (!completes_response && observation.has_value())) {
				throw std::runtime_error("Lifted facility action has the wrong visible image");
			}
			concrete_state_ = concrete_state_.Do(concrete_action.IndexedFlatten(), problem_->facility.bat, true);
			if (controller_->arena.bounds.active_domain != 0
				&& RenameableActiveDomainSize(concrete_state_) > controller_->arena.bounds.active_domain) {
				throw std::runtime_error("Lifted progression exceeds the synthesized active-domain bound");
			}
			abstract_state_ = edge.target;
			UpdateMappingForTarget(edge, true);

			const ArenaState concrete_source_control = RenameState(representative_source, source_mapping);
			const ArenaState concrete_target_control = RenameState(
				controller_->arena.states.at(abstract_state_), representative_to_concrete_);
			const FacilityProgramStateView source_view{&concrete_source_control.facility_control,
				&concrete_source_control.facility_bindings};
			const FacilityProgramStateView target_view{&concrete_target_control.facility_control,
				&concrete_target_control.facility_bindings};
			const uint64_t concrete_cost = problem_->facility.Cost(source_view, concrete_source,
				concrete_action, target_view, concrete_state_);
			if (concrete_cost != edge.cost) {
				throw std::runtime_error("Lifted facility action has a non-equivariant transition cost");
			}
			if (response_budget_ > controller_->optimal_response_cost
				|| edge.cost > controller_->optimal_response_cost - response_budget_) {
				throw std::runtime_error("Lifted response exceeds the synthesized budget");
			}
			response_budget_ += edge.cost;
			response.actions.emplace_back(std::move(concrete_action));
			response.cost = response_budget_;
		}
		if (controller_->arena.states.at(abstract_state_).owner == ArenaOwner::Lose) {
			throw std::runtime_error("Lifted controller reached the losing sink");
		}
		response.reached_goal = controller_->arena.states.at(abstract_state_).owner == ArenaOwner::Goal;
		pending_request_.reset();
		response_budget_ = 0;
		return response;
	}

	void ControllerSession::Stop() {
		if (controller_->arena.states.at(abstract_state_).owner != ArenaOwner::Environment) {
			throw std::logic_error("Stop is only valid at an Environment state");
		}
		for (const auto edge_id : controller_->arena.outgoing.at(abstract_state_)) {
			const auto& edge = controller_->arena.edges[edge_id];
			if (std::get_if<AuxiliaryLabel>(&edge.label)
				&& std::get<AuxiliaryLabel>(edge.label) == AuxiliaryLabel::Stop) {
				abstract_state_ = edge.target;
				return;
			}
		}
		throw std::invalid_argument("Recipe is not final at the current state");
	}

}
