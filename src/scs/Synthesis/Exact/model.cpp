#include "scs/Synthesis/Exact/model.h"

#include <algorithm>
#include <limits>
#include <set>
#include <stdexcept>

#include "scs/FirstOrderLogic/evaluator.h"
#include "scs/FirstOrderLogic/formula_utils.h"

namespace scs {
namespace {

	Formula IndexActionOccurrences(const Formula& formula, ResourceIndex resource) {
		if (const auto* action = std::get_if<Action>(&formula)) {
			Action indexed = *action;
			indexed.name = IndexedActionName(resource, indexed.name);
			return indexed;
		}
		if (const auto* action = std::get_if<CompoundAction>(&formula)) {
			CompoundAction indexed;
			for (const auto& component : action->Actions()) {
				Action copy = component;
				copy.name = IndexedActionName(resource, copy.name);
				indexed.AppendAction(std::move(copy));
			}
			return indexed;
		}
		if (const auto* unary = std::get_if<Box<UnaryConnective>>(&formula)) {
			return Box<UnaryConnective>{new UnaryConnective{
				IndexActionOccurrences((*unary)->child(), resource), (*unary)->kind()}};
		}
		if (const auto* binary = std::get_if<Box<BinaryConnective>>(&formula)) {
			return Box<BinaryConnective>{new BinaryConnective{
				IndexActionOccurrences((*binary)->lhs(), resource),
				IndexActionOccurrences((*binary)->rhs(), resource), (*binary)->kind()}};
		}
		if (const auto* quantifier = std::get_if<Box<Quantifier>>(&formula)) {
			return Box<Quantifier>{new Quantifier{(*quantifier)->variable(),
				IndexActionOccurrences((*quantifier)->child(), resource), (*quantifier)->kind()}};
		}
		return formula;
	}

	BasicActionTheory IndexedTheory(const BasicActionTheory& source, ResourceIndex resource) {
		BasicActionTheory result = source;
		result.successors.clear();
		for (const auto& [name, successor] : source.successors) {
			result.successors.emplace(name, Successor{successor.Terms(),
				IndexActionOccurrences(successor.Form(), resource), successor.IsLocalEffect()});
		}
		return result;
	}

	void MergeDynamicRelation(ankerl::unordered_dense::map<std::string, RelationalFluent>& target,
		const std::string& name, const RelationalFluent& source) {
		auto found = target.find(name);
		if (found == target.end()) {
			target.emplace(name, source);
			return;
		}
		if (found->second.Arity() != source.Arity()) {
			throw std::invalid_argument("Relational symbol '" + name + "' has inconsistent arity");
		}
		for (const auto& tuple : source.TrueTuples()) {
			found->second.AddValuation(tuple, true);
		}
	}

	void MergeTheoryData(BasicActionTheory& target, const BasicActionTheory& source,
		ankerl::unordered_dense::map<std::string, size_t>& successor_counts) {
		target.objects.insert(source.objects.begin(), source.objects.end());
		target.rigid_objects.insert(source.rigid_objects.begin(), source.rigid_objects.end());
		target.rigid.Merge(source.rigid);
		for (const auto& [name, relation] : source.Initial().Fluents()) {
			auto& declarations = target.initial_declarations.Declare(name, relation.Arity());
			for (const auto& tuple : relation.TrueTuples()) declarations.AddValuation(tuple, true);
		}
		target.initial_declarations.Merge(source.initial_declarations);

		Situation initial = target.Initial();
		for (const auto& [name, relation] : source.Initial().Fluents()) {
			MergeDynamicRelation(initial.relational_fluents_, name, relation);
		}
		target.SetInitial(std::move(initial));

		for (const auto& [name, successor] : source.successors) {
			++successor_counts[name];
			if (!target.successors.contains(name)) {
				target.successors.emplace(name, successor);
			}
		}
		for (const auto& [name, poss] : source.pre) {
			target.pre.try_emplace(name, poss);
		}
		for (const auto& [name, type] : source.types) {
			target.types.try_emplace(name, type);
		}
	}

	const Resource* FindResource(const Facility& facility, ResourceIndex index) {
		const auto found = std::ranges::find(facility.resources, index, &Resource::index);
		return found == facility.resources.end() ? nullptr : &*found;
	}

	bool EvaluateLocalPoss(const Poss& poss, const Action& action, const Situation& state,
		const BasicActionTheory& global, const ObjectSet& objects, DomainSemantics semantics) {
		if (poss.Terms().size() != action.terms.size()) return false;
		FirstOrderAssignment assignment;
		for (size_t i = 0; i < action.terms.size(); ++i) {
			const auto* object = std::get_if<Object>(&action.terms[i]);
			if (object == nullptr) return false;
			if (const auto* variable = std::get_if<Variable>(&poss.Terms()[i])) {
				assignment.Set(*variable, *object);
			}
		}
		Domain domain{state, global, objects};
		domain.semantics = semantics;
		return EvaluateFormula(poss.Form(), std::move(domain), assignment);
	}

}

	CompoundAction JointAction::Flatten() const {
		CompoundAction result;
		for (const auto& step : steps) {
			for (const auto& action : step.action.Actions()) result.AppendAction(action);
		}
		return result;
	}

	std::string IndexedActionName(ResourceIndex resource, std::string_view action_name) {
		return "@r" + std::to_string(resource) + ":" + std::string(action_name);
	}

	CompoundAction JointAction::IndexedFlatten() const {
		CompoundAction result;
		for (const auto& step : steps) {
			for (const auto& local : step.action.Actions()) {
				Action indexed = local;
				indexed.name = IndexedActionName(step.resource, local.name);
				result.AppendAction(std::move(indexed));
			}
		}
		return result;
	}

	Formula ResourceActionOccurs(ResourceIndex resource, const Action& action) {
		Action indexed = action;
		indexed.name = IndexedActionName(resource, action.name);
		return Box<BinaryConnective>{new BinaryConnective{Variable{"a"}, indexed, BinaryKind::Equal}};
	}

	Formula JointActionMatches(const std::vector<ResourceStep>& steps) {
		const CompoundAction complete = JointAction{steps}.IndexedFlatten();
		return Box<BinaryConnective>{new BinaryConnective{
			Variable{"a"}, complete, BinaryKind::Equal}};
	}

	bool JointAction::IsGround() const {
		for (const auto& step : steps) {
			for (const auto& action : step.action.Actions()) {
				for (const auto& term : action.terms) {
					if (!std::holds_alternative<Object>(term)) return false;
				}
			}
		}
		return true;
	}

	bool JointAction::AreAllNop() const {
		return std::ranges::all_of(steps, [](const ResourceStep& step) {
			return step.action.AreAllNop();
		});
	}

	std::ostream& operator<<(std::ostream& output, const ResourceStep& step) {
		return output << "r" << step.resource << ":" << step.action;
	}

	std::ostream& operator<<(std::ostream& output, const JointAction& action) {
		output << "(";
		for (size_t i = 0; i < action.steps.size(); ++i) {
			if (i != 0) output << ", ";
			output << action.steps[i];
		}
		return output << ")";
	}

	bool Facility::Possible(const JointAction& action, const Situation& state,
		const ObjectSet& objects, DomainSemantics semantics) const {
		if (action.steps.size() != resources.size() || !action.IsGround()) return false;
		std::set<ResourceIndex> named_resources;
		for (const auto& step : action.steps) {
			const Resource* resource = FindResource(*this, step.resource);
			if (resource == nullptr || !named_resources.emplace(step.resource).second
				|| step.action.Actions().empty()) return false;
			for (const auto& local : step.action.Actions()) {
				const auto found = resource->bat.pre.find(local.name);
				if (found == resource->bat.pre.end()
					|| found->second.Terms().size() != local.terms.size()) return false;
			}
		}
		if (callbacks.possible) return callbacks.possible(action, state);
		for (const auto& step : action.steps) {
			const Resource* resource = FindResource(*this, step.resource);
			for (const auto& local : step.action.Actions()) {
				const auto found = resource->bat.pre.find(local.name);
				if (!EvaluateLocalPoss(found->second, local, state, bat, objects, semantics)) {
					return false;
				}
			}
		}
		return true;
	}

	std::optional<CompoundAction> Facility::Observe(const JointAction& action) const {
		return callbacks.observe ? callbacks.observe(action) : std::nullopt;
	}

	uint64_t Facility::Cost(const FacilityProgramStateView& source_control,
		const Situation& source, const JointAction& action,
		const FacilityProgramStateView& target_control, const Situation& target) const {
		if (callbacks.cost) {
			return callbacks.cost(source_control, source, action, target_control, target);
		}
		uint64_t result = 1;
		for (const auto& step : action.steps) {
			const Resource* resource = FindResource(*this, step.resource);
			if (resource == nullptr) throw std::invalid_argument("Joint action names an unknown resource");
			for (const auto& local : step.action.Actions()) {
				const auto found = resource->bat.types.find(local.name);
				if (found == resource->bat.types.end()) {
					throw std::invalid_argument("Action '" + local.name + "' has no cost class");
				}
				const uint64_t component = found->second == ActionType::Manufacturing
					|| found->second == ActionType::Nop ? 1 : 2;
				if (component > std::numeric_limits<uint64_t>::max() - result) {
					throw std::overflow_error("Default facility transition cost overflows uint64_t");
				}
				result += component;
			}
		}
		return result;
	}

	Facility ComposeFacility(std::vector<Resource> resources, FacilityComposition composition) {
		Facility result;
		result.resources = std::move(resources);
		result.callbacks = std::move(composition.callbacks);
		ankerl::unordered_dense::map<std::string, size_t> successor_counts;
		MergeTheoryData(result.bat, composition.common, successor_counts);
		for (const auto& resource : result.resources) {
			MergeTheoryData(result.bat, IndexedTheory(resource.bat, resource.index), successor_counts);
		}

		for (const auto& [name, count] : successor_counts) {
			if (count > 1 && !composition.combined_successors.contains(name)) {
				throw std::invalid_argument("Shared fluent '" + name
					+ "' requires one facility-wide successor-state axiom");
			}
		}
		for (const auto& [name, successor] : composition.combined_successors) {
			result.bat.successors.insert_or_assign(name, successor);
		}
		for (const auto& [name, successor] : result.bat.successors) {
			if (!result.bat.Initial().Fluents().contains(name)) {
				Situation initial = result.bat.Initial();
				initial.AddFluent(name, RelationalFluent{successor.Terms().size()});
				result.bat.SetInitial(std::move(initial));
			}
		}

		const auto diagnostics = ValidateFacility(result);
		if (!diagnostics.empty()) throw std::invalid_argument(diagnostics.front());
		return result;
	}

	std::vector<std::string> ValidateFacility(const Facility& facility) {
		std::vector<std::string> errors;
		std::set<ResourceIndex> indices;
		for (const auto& resource : facility.resources) {
			if (!indices.emplace(resource.index).second) {
				errors.emplace_back("Duplicate resource index " + std::to_string(resource.index));
			}
			if (!resource.program) {
				errors.emplace_back("Resource " + std::to_string(resource.index) + " has no program");
			}
		}
		if (!facility.callbacks.observe) {
			errors.emplace_back("Facility requires an action-observation callback");
		}
		for (const Object& object : facility.bat.rigid_objects) {
			if (!object.IsRigid()) errors.emplace_back(
				"Rigid-object database contains a renameable identifier '" + object.name() + "'");
		}
		for (const auto& [name, fluent] : facility.bat.Initial().Fluents()) {
			const auto successor = facility.bat.successors.find(name);
			if (successor == facility.bat.successors.end()) {
				errors.emplace_back("Dynamic fluent '" + name + "' has no successor-state axiom");
			} else if (successor->second.Terms().size() != fluent.Arity()) {
				errors.emplace_back("Dynamic fluent '" + name + "' has inconsistent SSA arity");
			}
		}
		for (const auto& [name, relation] : facility.bat.rigid) {
			for (const auto& [tuple, value] : relation.ExplicitValuations()) {
				(void)value;
				if (std::ranges::any_of(tuple, [](const Object& object) { return object.IsIdentifier(); })) {
					errors.emplace_back("Rigid relation '" + name + "' contains a renameable identifier");
				} else if (std::ranges::any_of(tuple, [&](const Object& object) {
					return !facility.bat.rigid_objects.contains(object);
				})) {
					errors.emplace_back("Rigid relation '" + name + "' references an undeclared rigid constant");
				}
			}
		}
		return errors;
	}

}
