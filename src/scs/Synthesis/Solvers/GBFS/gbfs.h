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
#include "scs/Synthesis/Solvers/GBFS/gbfs_comparator.h"

#include "scs/Common/timer.h"
#include "scs/Combinatorics/Utils/duplicates.h"

#ifdef max
	#undef max
#endif

namespace scs {

	using TransitionType = nightly::Transition<CgState, CgTransition>;

	struct GBFS {
	public:
		const std::span<CharacteristicGraph>& resource_graphs;
		const CharacteristicGraph& recipe_graph;
		const BasicActionTheory& global_bat;
		const Limits& lim;
		ITopology& topology;

		CompoundActionCache action_cache;
		Candidate best_candidate_;
	public:
		GBFS(const std::span<CharacteristicGraph>& resource_graphs, const CharacteristicGraph& recipe_graph,
		const BasicActionTheory& global_bat, ITopology& topology,
		const Limits& lim = Limits())
		: resource_graphs(resource_graphs), recipe_graph(recipe_graph),
		global_bat(global_bat), topology(topology), lim(lim),
		action_cache(global_bat.objects) {
			best_candidate_.total_cost = std::numeric_limits<int32_t>::max();
		}

		std::vector<Candidate> Advance(Candidate& cand, bool& first_generated) {
			std::vector<Candidate> ret;
			Stage current_stage = std::move(cand.stages.front());
			cand.stages.pop();
			if (!WithinLimits(cand, current_stage, lim)) {
				if (current_stage.type == StageType::Regular) {
					return ret;
				} else if (current_stage.type == StageType::Pi) {
					// Skippable pi instantiation stage if it cannot be found. Will still need to end in 'Final' elsewhere.
					ret.emplace_back(cand);
					return ret;
				}
			}
			const auto& target_ca = current_stage.recipe_transition.label().act;

			for (const auto& trans : topology.at(*current_stage.resource_states).transitions()) {
				for (const auto& concrete_ca : action_cache.Get(trans.label().act)) {
					if (concrete_ca.AreAllNop()) {
						continue;
					}
					if (!Legal(concrete_ca, target_ca, global_bat)) {
						continue;
					}
					if (!current_stage.sit.Possible(concrete_ca, global_bat) || !Holds(current_stage, trans.label().condition, global_bat)) {
						continue;
					}

					Candidate next_cand = cand;
					Stage next_stage = current_stage;
					next_stage.sit = next_stage.sit.Do(concrete_ca, global_bat);
					next_stage.resource_states = &trans.to();

					AddControllerTransition(next_cand, next_stage, { concrete_ca, trans.label().condition }, current_stage);
					UpdateCost(next_cand, next_stage, global_bat, concrete_ca, target_ca);

					SCS_INFO(fmt::format(fmt::fg(fmt::color::cyan),
						"Action {} vs {}", concrete_ca, target_ca));

					// Facility has completed recipe action
					if (UnifyActions(concrete_ca, target_ca)) {
						SCS_INFO(fmt::format(fmt::fg(fmt::color::gold),
							"Found facility action {}", concrete_ca));
						next_cand.completed_recipe_transitions++;

						if (recipe_graph.lts.at(next_stage.recipe_transition.to()).transitions().empty()) {
							// No transitions in next state
							if (Holds(next_stage, next_stage.recipe_transition.to().final_condition, global_bat)) {
								// Final state
								if (next_cand.stages.empty()) {
									// All recipe transitions processed for candidate, potentially update best
									UpdateBest(next_cand, first_generated, best_candidate_);
									continue;
								} else {
									// More recipe transitions need to be processed
									ret.emplace_back(next_cand);
									continue;
								}
							} else {
								// Entering a state which is not final but has no transitions
								SCS_CRITICAL("FNT");
							}
						} else { // Next recipe state has transitions to do, add all possible transitions
							NextStages(next_cand, next_stage, recipe_graph, global_bat, lim, &trans.to(), action_cache.SimpleExecutor());
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
			std::priority_queue<Candidate, std::vector<Candidate>, GreedyCandidateComparator> pq;

			Candidate initial_candidate = CreateInitialCandidate(global_bat, resource_graphs, topology, recipe_graph,
				action_cache.SimpleExecutor());
			pq.push(initial_candidate);

			while (!pq.empty() && !first_generated) {
				Candidate cand = std::move(pq.top());
				pq.pop();
				SCS_INFO(fmt::format(fmt::fg(fmt::color::orchid), 
					"Popping with completed transitions {}", cand.completed_recipe_transitions));

				auto next = Advance(cand, first_generated);
				for (const auto& c : next) {
					pq.push(c);
				}
			}
			if (best_candidate_.total_cost != std::numeric_limits<int32_t>::max()) {
				SCS_INFO("Greedy controller, cost = {}, num transitions = {}", best_candidate_.total_cost, best_candidate_.total_transitions);
				return best_candidate_;
			} else {
				SCS_INFO("Was unable to find any controller for the recipe and resources provided");
				return std::nullopt;
			}
		}

	};


}

