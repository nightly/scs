#include "core.h"

#include <vector>
#include <span>
#include <algorithm>
#include <functional>
#include <stdexcept>

#include "scs/Synthesis/Solvers/Core/candidate.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/Synthesis/Solvers/Heuristics/heuristics.h"
#include "scs/Synthesis/Topology/types.h"
#include "scs/Synthesis/Topology/interface_topology.h"
#include "scs/FirstOrderLogic/evaluator.h"
#include "scs/SituationCalculus/object_universe.h"

namespace scs {
	namespace {

		void AddActionConstants(ObjectSet& objects, const CompoundAction& action) {
			for (const auto& component : action.Actions()) {
				for (const auto& term : component.terms) {
					if (const auto* object = std::get_if<Object>(&term)) {
						objects.emplace(*object);
					}
				}
			}
		}

		Action GroundAction(const Action& action, const FirstOrderAssignment& bindings) {
			Action result = action;
			for (auto& term : result.terms) {
				if (const auto* variable = std::get_if<Variable>(&term)) {
					term = bindings.GetObject(*variable);
				}
			}
			return result;
		}

		CompoundAction GroundAction(const CompoundAction& action,
			const FirstOrderAssignment& bindings) {
			CompoundAction result;
			for (const auto& component : action.Actions()) {
				result.AppendAction(GroundAction(component, bindings));
			}
			return result;
		}

		std::vector<Object> OrderedObjects(const ObjectSet& objects) {
			std::vector<Object> result(objects.begin(), objects.end());
			std::ranges::sort(result, {}, &Object::name);
			return result;
		}

	}

	ObjectSet StageSupport(const Stage& stage, const BasicActionTheory& bat) {
		auto objects = RelevantObjects(stage.sit, bat);
		AddAssignmentObjects(objects, stage.recipe_bindings);
		for (const auto& bindings : stage.resource_bindings) {
			AddAssignmentObjects(objects, bindings);
		}
		AddGroundActionObjects(objects, stage.recipe_transition.label().act);
		return objects;
	}

	std::vector<BoundCgTransition> InstantiateTransition(const CgTransition& transition,
		const FirstOrderAssignment& source_bindings, const Situation& situation,
		const BasicActionTheory& bat, const ObjectSet& objects) {
		ObjectSet complete_objects = objects;
		AddActionConstants(complete_objects, transition.act);
		const auto ordered_objects = OrderedObjects(complete_objects);
		std::vector<FirstOrderAssignment> assignments{source_bindings};
		for (const auto& variable : transition.vars) {
			std::vector<FirstOrderAssignment> extended;
			extended.reserve(assignments.size() * ordered_objects.size());
			for (const auto& assignment : assignments) {
				for (const auto& object : ordered_objects) {
					extended.emplace_back(assignment.Extended(variable, object));
				}
			}
			assignments = std::move(extended);
		}

		std::vector<BoundCgTransition> result;
		for (const auto& assignment : assignments) {
			Evaluator evaluator{{situation, bat, bat.CoopMx(), bat.RoutesMx(), complete_objects}, assignment};
			if (!std::visit(evaluator, transition.condition)) {
				continue;
			}
			CgTransition grounded = transition;
			grounded.act = GroundAction(transition.act, assignment);
			grounded.vars.clear();
			result.emplace_back(BoundCgTransition{std::move(grounded), assignment});
		}
		return result;
	}

	std::vector<BoundTopologyStep> InstantiateTopologyTransition(
		const nightly::Transition<TopologyState, TopologyTransition>& transition,
		const Stage& stage, const BasicActionTheory& bat) {
		if (transition.label().components.size() != transition.to().size()) {
			throw std::invalid_argument("Topology transition does not retain one component transition per resource");
		}
		if (stage.resource_bindings.size() != transition.label().components.size()) {
			throw std::invalid_argument("Stage does not carry one binding environment per resource");
		}

		auto objects = StageSupport(stage, bat);
		for (const auto& component : transition.label().components) {
			AddActionConstants(objects, component.label().act);
		}
		std::vector<std::vector<BoundCgTransition>> component_steps;
		component_steps.reserve(transition.label().components.size());
		for (size_t i = 0; i < transition.label().components.size(); ++i) {
			component_steps.emplace_back(InstantiateTransition(transition.label().components[i].label(),
				stage.resource_bindings[i], stage.sit, bat, objects));
			if (component_steps.back().empty()) {
				return {};
			}
		}

		std::vector<BoundTopologyStep> result;
		BoundTopologyStep current;
		current.resource_bindings.resize(component_steps.size());
		const auto combine = [&](this const auto& self, size_t component) -> void {
			if (component == component_steps.size()) {
				result.emplace_back(current);
				return;
			}
			for (const auto& step : component_steps[component]) {
				const size_t action_count = current.action.Actions().size();
				for (const auto& action : step.transition.act.Actions()) {
					current.action.AppendAction(action);
				}
				current.resource_bindings[component] =
					step.bindings.Project(transition.to()[component].live_variables);
				self(component + 1);
				current.action.Actions().resize(action_count);
			}
		};
		combine(0);
		return result;
	}

	bool Holds(const Situation& situation, const Formula& form, const BasicActionTheory& bat,
		const FirstOrderAssignment& assignment, const ObjectSet& objects) {
		scs::Evaluator eval{{situation, bat, bat.CoopMx(), bat.RoutesMx(), objects}, assignment};
		return std::visit(eval, form);
	}

	bool Holds(const Stage& stage, const Formula& form, const BasicActionTheory& bat) {
		return Holds(stage.sit, form, bat, stage.recipe_bindings, StageSupport(stage, bat));
	}

	size_t AddControllerTransition(Candidate& candidate, Stage& next_stage, const PlanTransition& trans, const Stage& previous_stage) {
		candidate.explored[previous_stage.recipe_transition.label().Id()]++;

		size_t next_n = candidate.counter.Increment();
		candidate.plan.lts.AddTransition(previous_stage.plan_lts_state, trans, next_n);
		next_stage.plan_lts_state = next_n;
		return next_n;
	}

	size_t TotalRecipeTransitions(const CharacteristicGraph& cg) {
		return cg.lts.NumOfTransitions();
	}

	bool WithinLimits(const Candidate& cand, const Stage& stage, const Limits& lim) {
		if (stage.local_transitions >= lim.stage_transition_limit) {
			return false;
		}
		if (stage.local_cost >= lim.stage_cost_limit) {
			return false;
		}

		if (cand.total_transitions >= lim.global_transition_limit) {
			return false;
		}
		if (cand.total_cost >= lim.global_cost_limit) {
			return false;
		}
		return true;
	}

	void UpdateBest(const Candidate& cand, Candidate& best_candidate) {
		if (cand.total_cost < best_candidate.total_cost) {
			best_candidate = cand;
		}
	}

	Candidate CreateInitialCandidate(const BasicActionTheory& bat, const std::span<CharacteristicGraph>& resource_graphs,
	const ITopology& topology, const CharacteristicGraph& recipe_graph) {
		(void)resource_graphs;
		Candidate ret;
		ret.plan.lts.set_initial_state(0);
		const Situation& s0 = bat.Initial();
		const auto objects = RelevantObjects(s0, bat);
		for (const auto& recipe_trans : recipe_graph.lts.at(0).transitions()) {
			for (const auto& bound : InstantiateTransition(recipe_trans.label(), {}, s0, bat, objects)) {
				Stage stage;
				stage.recipe_transition = recipe_trans;
				stage.recipe_transition.label() = bound.transition;
				stage.recipe_bindings = bound.bindings;
				stage.resource_bindings.resize(topology.initial_state().size());
				stage.sit = s0;
				stage.resource_states = &topology.initial_state();
				stage.local_transitions = 0;
				stage.plan_lts_state = 0;
				stage.type = recipe_trans.label().HasVariables() ? StageType::Pi : StageType::Regular;
				ret.stages.emplace(stage);
			}
		}
		return ret;
	}


	void NextStages(Candidate& next_candidate, const Stage& old_stage, const CharacteristicGraph& recipe_graph, 
	const BasicActionTheory& bat, const Limits& lim, const TopologyState* next_resource_states) {
		const auto source_bindings = old_stage.recipe_bindings.Project(
			old_stage.recipe_transition.to().live_variables);
		auto objects = RelevantObjects(old_stage.sit, bat);
		AddAssignmentObjects(objects, source_bindings);
		for (const auto& bindings : old_stage.resource_bindings) {
			AddAssignmentObjects(objects, bindings);
		}
		AddGroundActionObjects(objects, old_stage.recipe_transition.label().act);
		for (const auto& recipe_trans : recipe_graph.lts.at(old_stage.recipe_transition.to()).transitions()) {
			if (next_candidate.explored.contains(recipe_trans.label().Id())) {
				if (next_candidate.explored.at(recipe_trans.label().Id()) >= lim.fairness_limit) {
					continue;
				}
			}
			for (const auto& bound : InstantiateTransition(recipe_trans.label(), source_bindings,
				old_stage.sit, bat, objects)) {
				Stage future_stage;
				future_stage.recipe_transition = recipe_trans;
				future_stage.recipe_transition.label() = bound.transition;
				future_stage.recipe_bindings = bound.bindings;
				future_stage.resource_bindings = old_stage.resource_bindings;
				future_stage.resource_states = next_resource_states;
				future_stage.local_transitions = 0;
				future_stage.local_cost = 0;
				future_stage.sit = old_stage.sit;
				future_stage.plan_lts_state = old_stage.plan_lts_state;
				future_stage.type = recipe_trans.label().HasVariables() ? StageType::Pi : StageType::Regular;

				SCS_INFO(fmt::format(fmt::fg(fmt::color::hot_pink),
					"Now looking for action {}", future_stage.recipe_transition.label().act));
				next_candidate.stages.emplace(future_stage);
			}

		}
	}


}
