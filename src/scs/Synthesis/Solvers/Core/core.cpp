#include "core.h"

#include <vector>
#include <span>

#include "scs/Synthesis/Solvers/Core/candidate.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/Synthesis/Solvers/Heuristics/heuristics.h"

namespace scs {

	Candidate CreateInitialCandidate(const BasicActionTheory& bat, const std::span<CharacteristicGraph>& resource_graphs,
	const CharacteristicGraph& recipe_graph) {
		Candidate ret;
		ret.plan.lts.set_initial_state(0);
		const Situation& s0 = bat.Initial();
		std::vector<CgState> res_states(resource_graphs.size(), CgState{ 0 });
		for (const auto& transition : recipe_graph.lts.at(0).transitions()) {
			Stage stage;
			stage.recipe_transition = &transition;
			stage.sit = s0;
			stage.resource_states = res_states;
			stage.local_transitions = 0;
			ret.stages.emplace(stage);
		}
		return ret;
	}

	bool Holds(const Stage& stage, const Formula& form, const BasicActionTheory& bat) {
		scs::Evaluator eval{ {stage.sit, bat, bat.CoopMx(), bat.RoutesMx()} };
		return std::visit(eval, form);
	}

	size_t AddControllerTransition(Candidate& candidate, Stage& next_stage, const PlanTransition& trans, const Stage& previous_stage) {
		candidate.explored[previous_stage.recipe_transition->label().Id()]++;

		size_t next_n = candidate.counter.Increment();
		candidate.plan.lts.AddTransition(previous_stage.plan_state, trans, next_n);
		next_stage.plan_state = next_n;
		return next_n;
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

	void NextStages(Candidate& next_candidate, const Stage& old_stage, const CharacteristicGraph& recipe_graph, 
	const BasicActionTheory& bat, const Limits& lim) {
		for (const auto& recipe_trans : recipe_graph.lts.at(old_stage.recipe_transition->to()).transitions()) {
			if (!Holds(old_stage, recipe_trans.label().condition, bat)) {
				continue;
			}
			if (next_candidate.explored.contains(recipe_trans.label().Id())) {
				if (next_candidate.explored.at(recipe_trans.label().Id()) >= lim.fairness_limit) {
					continue;
				}
			}
			Stage future_stage;
			future_stage.recipe_transition = &recipe_trans;
			future_stage.sit = old_stage.sit;
			future_stage.resource_states = old_stage.resource_states;
			future_stage.plan_state = old_stage.plan_state;
			future_stage.local_transitions = 0;

			SCS_INFO(fmt::format(fmt::fg(fmt::color::hot_pink),
				"Now looking for action {}", future_stage.recipe_transition->label().act));
			next_candidate.stages.emplace(future_stage);
		}
	}


	void UpdateBest(const Candidate& cand, bool& first_generated, Candidate& best_candidate) {
		first_generated = true;
		if (cand.total_cost < best_candidate.total_cost) {
			best_candidate = cand;
		}
	}



}