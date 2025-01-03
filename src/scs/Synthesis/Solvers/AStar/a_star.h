#pragma once

#include <optional>
#include <span>
#include <limits>
#include <cstdint>
#include <random>

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

	struct AStar {
	public:
		const std::span<CharacteristicGraph>& resource_graphs;
		const CharacteristicGraph& recipe_graph;
		const BasicActionTheory& global_bat;
		const Limits& lim;
		ITopology& topology;
		bool first_generated_ = false;

		//std::random_device rd;
		//std::mt19937 rng_{ rd() };

		Cache cache_; // Handles ungrounded actions into grounded actions & stores fluents -> actions

		Candidate best_candidate_;
		
		#if (SCS_STATS_OUTPUT == 1 || SCS_MINIMAL_STATS == 1)
			size_t visited_situations_ = 0;
		#endif
	public:
		AStar(const std::span<CharacteristicGraph>& resource_graphs, const CharacteristicGraph& recipe_graph,
		const BasicActionTheory& global_bat, ITopology& topology, 
		const Limits& lim = Limits())
		: resource_graphs(resource_graphs), recipe_graph(recipe_graph),
		global_bat(global_bat), topology(topology), lim(lim),
		cache_(global_bat.objects) {
			best_candidate_.total_cost = std::numeric_limits<int32_t>::max();
		}

		std::vector<Candidate> Advance(Candidate& cand) {
			std::vector<Candidate> ret;
			Stage current_stage = std::move(cand.stages.front());
			cand.stages.pop();
			if (!WithinLimits(cand, current_stage, lim)) {
				return ret;
			}
			const auto& target_ca = current_stage.recipe_transition.label().act;

			for (const auto& trans : topology.at(*current_stage.resource_states).transitions()) {
				for (const auto& concrete_ca : cache_.Get(trans.label().act)) {
					if (concrete_ca.AreAllNop()) {
						continue;
					}
					if (!Legal(concrete_ca, target_ca, global_bat)) {
						continue;
					}
					if (!current_stage.sit.Possible(concrete_ca, global_bat) || !Holds(current_stage, trans.label().condition, global_bat)) {
						continue;
					}
					#if (SCS_STATS_OUTPUT == 1 || SCS_MINIMAL_STATS == 1)
						visited_situations_++;
					#endif

					Candidate next_cand = cand;
					Stage next_stage = current_stage;
					next_stage.sit = next_stage.sit.Do(concrete_ca, global_bat);
					next_stage.resource_states = &trans.to();

					AddControllerTransition(next_cand, next_stage, {concrete_ca, trans.label().condition}, current_stage);
					UpdateCost(next_cand, next_stage, global_bat, concrete_ca, target_ca);

					// Facility has completed recipe action
					if (UnifyActions(concrete_ca, target_ca)) {
						next_cand.completed_recipe_transitions++;
						SCS_INFO(fmt::format(fmt::fg(fmt::color::gold),
							"Found facility action {} for {} [{}]", concrete_ca, target_ca, next_cand.completed_recipe_transitions));

						if (!recipe_graph.lts.at(next_stage.recipe_transition.to()).transitions().empty()) {
							NextStages(next_cand, next_stage, recipe_graph, global_bat, lim, &trans.to(),
								cache_.SimpleExecutor());
							ret.emplace_back(next_cand);
							continue;
						} else {
							if (!Holds(next_stage, next_stage.recipe_transition.to().final_condition, global_bat)) {
								// Since next state has no transitions - it is a Final state and
								// Final must hold for a terminating recipe. If it does not, we discard the candidate.
								continue;
							}
						}

						if (next_cand.stages.empty()) {
							/* All recipe transitions processed for candidate, potentially update best
							No other stages in the queue. We assume recipes must be terminating already.
							Any candidate that failed to simulate a legal transition would be disregarded 
							with WithinLimits check, so every required transition has already been done.
							*/
							first_generated_ = true;
							UpdateBest(next_cand, best_candidate_);
							continue;
						} else {
							// The next stages are not empty, so we continue with this candidate
							ret.emplace_back(next_cand);
							continue;
						}
					} else { // Not unified recipe action, continue current stage
						SCS_INFO(fmt::format(fmt::fg(fmt::color::cyan),
							"Action {} vs {}", concrete_ca, target_ca));
						next_cand.stages.emplace(next_stage);
						ret.emplace_back(std::move(next_cand));
					}
				}
			}

			return ret;
		}

		std::optional<Candidate> Synthethise() {
			#if (SCS_STATS_OUTPUT == 1)
				visited_situations_ = 0;
			#endif

			first_generated_ = false;
			std::priority_queue<Candidate, std::vector<Candidate>, CandidateComparator> pq;

			Candidate initial_candidate = CreateInitialCandidate(global_bat, resource_graphs, topology, recipe_graph,
				cache_.SimpleExecutor());
			pq.push(initial_candidate);

			while (!pq.empty() && (best_candidate_.total_cost > pq.top().total_cost || !first_generated_)) {
				Candidate cand = std::move(pq.top());
				pq.pop();

				auto next = Advance(cand);
				for (const auto& c : next) {
					pq.push(c);
				}
			}
			if (best_candidate_.total_cost != std::numeric_limits<int32_t>::max()) {
				SCS_INFOSTATS("AStar controller, cost = {}, num transitions = {}", best_candidate_.total_cost, best_candidate_.total_transitions);
				
				#if (SCS_STATS_OUTPUT == 1)
					SCS_STATS("Number of action considerations = {}", cache_.SizeComplete());
					SCS_STATS("Number of visited situations = {}", visited_situations_);
					SCS_STATS("Number of topology states = {}, number of topology transitions = {}", topology.lts().NumOfStates(),
						topology.lts().NumOfTransitions());
				#endif
				#if (SCS_MINIMAL_STATS == 1)
					SCS_MINSTATS("Number of visited situations = {}", visited_situations_);
				#endif

				return best_candidate_;
			} else {
				SCS_CRITICAL("Was unable to find any controller for the recipe and resources provided");
				return std::nullopt;
			}
		}
	
	};
	

}

