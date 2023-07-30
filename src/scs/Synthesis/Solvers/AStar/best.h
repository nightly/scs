#pragma once

#include <optional>
#include <span>
#include <limits>
#include <cstdint>

#include "scs/Synthesis/plan.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/SituationCalculus/bat.h"
#include "scs/Synthesis/Actions/unify.h"
#include "scs/Synthesis/Solvers/AStar/valuation.h"
#include "scs/Synthesis/Solvers/AStar/limits.h"
#include "scs/Synthesis/Solvers/AStar/candidate.h"
#include "scs/Synthesis/Topology/topology.h"
#include "scs/Synthesis/Actions/cache.h"
#include "scs/Combinatorics/Utils/duplicates.h"
#include "scs/Synthesis/export.h"

#ifdef max
	#undef max
#endif

namespace scs {

	using TransitionType = nightly::Transition<CgState, CgTransition>;

	struct Best {
	public:
		const std::span<CharacteristicGraph>& resource_graphs;
		const CharacteristicGraph& recipe_graph;
		const BasicActionTheory& global_bat;
		const Limits& lim;
		ITopology& topology;
		int64_t h_num;

		CompoundActionCache action_cache;
		Candidate best_candidate;
	public:
		Best(const std::span<CharacteristicGraph>& resource_graphs, const CharacteristicGraph& recipe_graph,
		const BasicActionTheory& global_bat, ITopology& topology, 
		const Limits& lim = Limits(), int64_t h_num = 1)
		: resource_graphs(resource_graphs), recipe_graph(recipe_graph),
		global_bat(global_bat), topology(topology), lim(lim),
		action_cache(global_bat.objects), h_num(h_num) {
			best_candidate.total_cost = std::numeric_limits<int64_t>::max();
		}

		Candidate CreateInitialCandidate() const {
			Candidate ret;
			ret.plan.lts.set_initial_state(0);
			const Situation& s0 = global_bat.Initial();
			std::vector<CgState> res_states(resource_graphs.size(), CgState{0});
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

		size_t AddControllerTransition(Candidate& candidate, Stage& next_stage, const RecipeTransition& trans, const Stage& previous_stage) const {
			size_t next_n = candidate.counter.Increment();
			candidate.plan.lts.AddTransition(previous_stage.plan_state, trans, next_n);
			next_stage.plan_state = next_n;
			return next_n;
		}

		void UpdateBest(const Candidate& cand, bool& first_generated) {
			first_generated = true;
			if (cand.total_cost < best_candidate.total_cost) {
				best_candidate = cand;
			}
		}

		bool WithinLimits(const Candidate& cand, const Stage& stage) const {
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

		bool Holds(const Stage& stage, const Formula& form) const {
			scs::Evaluator eval{ {stage.sit, global_bat, global_bat.CoopMx(), global_bat.RoutesMx()} };
			return std::visit(eval, form);
		}

		void NextStages(Candidate& next_candidate, const Stage& old_stage) {
			for (const auto& recipe_trans : recipe_graph.lts.at(old_stage.recipe_transition->to()).transitions()) {
				if (!Holds(old_stage, recipe_trans.label().condition)) {
					continue;
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

		std::vector<Candidate> Advance(Candidate& cand, bool& first_generated) {
			std::vector<Candidate> ret;
			Stage prior_stage = std::move(cand.stages.front());
			cand.stages.pop();
			if (!WithinLimits(cand, prior_stage)) {
				return ret;
			}
			
			for (const auto& trans : topology.at(prior_stage.resource_states).transitions()) {
				for (const auto& concrete_ca : action_cache.Get(trans.label().act)) {
					Candidate next_cand = cand;
					Stage next_stage = prior_stage;

					if (next_stage.sit.Possible(concrete_ca, global_bat) && Holds(next_stage, trans.label().condition)) {
						next_stage.sit = next_stage.sit.Do(concrete_ca, global_bat);
					} else {
						continue;
					}
					SCS_TRACE(fmt::format(fmt::fg(fmt::color::lavender_blush),
						"Stage local {}", next_stage.local_num));

					auto next_state = AddControllerTransition(next_cand, next_stage, {concrete_ca, trans.label().condition}, prior_stage);
					next_stage.resource_states = trans.to();
					UpdateCost(next_cand, next_stage, concrete_ca);

					SCS_INFO(fmt::format(fmt::fg(fmt::color::cyan),
						"Action {} vs {}", concrete_ca, prior_stage.recipe_transition->label().act));

					// Facility has completed recipe action
					if (UnifyActions(prior_stage.recipe_transition->label().act, concrete_ca)) {
						SCS_INFO(fmt::format(fmt::fg(fmt::color::gold),
							"Found facility action {}", concrete_ca));
						if (recipe_graph.lts.at(prior_stage.recipe_transition->to()).transitions().empty()) {
							// No transitions in next state
							// @Incomplete: check if condition holds in such transitions not just final = true
							if (prior_stage.recipe_transition->to().final_condition == scs::Formula{true}) {
								if (next_cand.stages.empty()) {
									// No more stages left in candidate other than this,
									// the next recipe transition has no further transitions
									// i.e. the candidate could be complete 
									next_cand.completed_recipe_transitions += h_num;
									UpdateBest(next_cand, first_generated);
									continue;
								} else {
									// Next state is final, no transitions, but more overall stages still left
									next_cand.completed_recipe_transitions += h_num;
									ret.emplace_back(next_cand);
									continue;
								}
							} else {
								// Entering a state which is not final but has no transitions
								SCS_CRITICAL("FNT");
							}
						} else { // Next recipe state has transitions to do
							next_cand.completed_recipe_transitions += h_num;
							NextStages(next_cand, next_stage);
							ret.emplace_back(next_cand);
							continue;
						}
					} else { // Not unified recipe action, continue current stage
						next_cand.stages.push(next_stage);
						ret.emplace_back(std::move(next_cand));
					}
				}
			}

			return ret;
		}

		std::optional<Plan> Synthethise(bool quick = false) {
			bool first_generated = false;
			std::priority_queue<Candidate, std::vector<Candidate>, CandidateComparator> pq;

			Candidate initial_candidate = CreateInitialCandidate();
			pq.push(initial_candidate);

			while (!pq.empty() && (best_candidate.total_cost > pq.top().total_cost || !first_generated)) {
				if (first_generated && quick) {
					break;
				}
				Candidate cand = std::move(pq.top());
				pq.pop();

				auto next = Advance(cand, first_generated);
				for (const auto& c : next) {
					pq.push(c);
				}
			}
			SCS_INFO("Best controller, cost = {}, num transitions = {}", best_candidate.total_cost, best_candidate.total_transitions);
			return best_candidate.plan;
		}
	
	};
	

}

// @Todo:
// - check recipe transition conditions better emplacing

