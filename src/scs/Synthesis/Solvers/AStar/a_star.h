#pragma once

#include <optional>
#include <span>
#include <limits>
#include <cstdint>

#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/SituationCalculus/bat.h"
#include "scs/Synthesis/Plan/plan.h"
#include "scs/Synthesis/Actions/unify.h"
#include "scs/Synthesis/Solvers/Heuristics/heuristics.h"
#include "scs/Synthesis/Solvers/Core/candidate.h"
#include "scs/Synthesis/Topology/topology.h"
#include "scs/Synthesis/Actions/cache.h"
#include "scs/Synthesis/Plan/export.h"
#include "scs/Synthesis/Solvers/Core/core.h"

#include "scs/Common/timer.h"
#include "scs/Combinatorics/Utils/duplicates.h"

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

		CompoundActionCache action_cache;
		Candidate best_candidate_;
	public:
		Best(const std::span<CharacteristicGraph>& resource_graphs, const CharacteristicGraph& recipe_graph,
		const BasicActionTheory& global_bat, ITopology& topology, 
		const Limits& lim = Limits())
		: resource_graphs(resource_graphs), recipe_graph(recipe_graph),
		global_bat(global_bat), topology(topology), lim(lim),
		action_cache(global_bat.objects) {
			best_candidate_.total_cost = std::numeric_limits<int32_t>::max();
		}

		std::vector<Candidate> Advance(Candidate& cand, bool& first_generated) {
			std::vector<Candidate> ret;
			Stage prior_stage = std::move(cand.stages.front());
			cand.stages.pop();
			if (!WithinLimits(cand, prior_stage, lim)) {
				return ret;
			}
			const auto& target_ca = prior_stage.recipe_transition->label().act;

			for (const auto& trans : topology.at(*prior_stage.resource_states).transitions()) {
				for (const auto& concrete_ca : action_cache.Get(trans.label().act)) {
					if (concrete_ca.AreAllNop()) {
						continue;
					}
					if (!Legal(concrete_ca, target_ca, global_bat)) {
						continue;
					}
					Candidate next_cand = cand;
					Stage next_stage = prior_stage;

					if (next_stage.sit.Possible(concrete_ca, global_bat) && Holds(next_stage, trans.label().condition, global_bat)) {
						next_stage.sit = next_stage.sit.Do(concrete_ca, global_bat);
					} else {
						continue;
					}

					auto next_state = AddControllerTransition(next_cand, next_stage, {concrete_ca, trans.label().condition}, prior_stage);
					next_stage.resource_states = &trans.to();
					UpdateCost(next_cand, next_stage, global_bat, concrete_ca, target_ca);

					SCS_INFO(fmt::format(fmt::fg(fmt::color::cyan),
						"Action {} vs {}", concrete_ca, target_ca));

					// Facility has completed recipe action
					if (UnifyActions(concrete_ca, target_ca)) {
						SCS_INFO(fmt::format(fmt::fg(fmt::color::gold),
							"Found facility action {}", concrete_ca));
						if (recipe_graph.lts.at(prior_stage.recipe_transition->to()).transitions().empty()) {
							// No transitions in next state
							if (Holds(next_stage, prior_stage.recipe_transition->to().final_condition, global_bat)) {
								// Final state
								if (next_cand.stages.empty()) {
									// No more stages left to process in the overall candidate
									next_cand.completed_recipe_transitions++;
									UpdateBest(next_cand, first_generated, best_candidate_);
									continue;
								} else {
									// Next state is final, no transitions, but more overall stages still left
									next_cand.completed_recipe_transitions++;
									ret.emplace_back(next_cand);
									continue;
								}
							} else {
								// Entering a state which is not final but has no transitions
								SCS_CRITICAL("FNT");
							}
						} else { // Next recipe state has transitions to do, add all possible transitions
							next_cand.completed_recipe_transitions++;
							NextStages(next_cand, next_stage, recipe_graph, global_bat, lim, &trans.to());
							ret.emplace_back(next_cand);
							continue;
						}
					} else { // Not unified recipe action, continue current stage
						next_cand.stages.emplace(next_stage);
						ret.emplace_back(std::move(next_cand));
					}
				}
			}

			return ret;
		}

		std::optional<Candidate> Synthethise() {
			bool first_generated = false;
			std::priority_queue<Candidate, std::vector<Candidate>, CandidateComparator> pq;

			Candidate initial_candidate = CreateInitialCandidate(global_bat, resource_graphs, topology, recipe_graph);
			pq.push(initial_candidate);

			while (!pq.empty() && (best_candidate_.total_cost > pq.top().total_cost || !first_generated)) {
				Candidate cand = std::move(pq.top());
				pq.pop();

				auto next = Advance(cand, first_generated);
				for (const auto& c : next) {
					pq.push(c);
				}
			}
			if (best_candidate_.total_cost != std::numeric_limits<int32_t>::max()) {
				SCS_INFO("Best controller, cost = {}, num transitions = {}", best_candidate_.total_cost, best_candidate_.total_transitions);
				return best_candidate_;
			} else {
				SCS_INFO("Was unable to find any controller for the recipe and resources provided");
				return std::nullopt;
			}
		}
	
	};
	

}

