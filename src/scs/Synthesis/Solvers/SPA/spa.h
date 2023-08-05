#pragma once

#include <optional>
#include <span>
#include <limits>
#include <cstdint>
#include <atomic>
#include <thread>
#include <concurrent_priority_queue.h>

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
#include "scs/Synthesis/Actions/pre_expand.h"

#include "scs/Common/timer.h"
#include "scs/Combinatorics/Utils/duplicates.h"

#ifdef max
	#undef max
#endif

#define SPDLOG_DISABLE_TID_CACHING
#define SPDLOG_NO_TLS

namespace scs {

	using TransitionType = nightly::Transition<CgState, CgTransition>;

	// Simple Parallel A*
	struct SPA {
	public:
		const std::span<CharacteristicGraph>& resource_graphs;
		const CharacteristicGraph& recipe_graph;
		const BasicActionTheory& global_bat;
		const Limits& lim;
		CompleteTopology& topology;

		CompoundActionCache action_cache_; // Must be pre-expanded
		Concurrency::concurrent_priority_queue<Candidate, CandidateComparator> pq_;

		size_t num_threads_;

		std::atomic<bool> first_generated_{false};
		std::atomic<int32_t> best_cost_;
		Candidate best_candidate_;
		std::mutex best_mutex_;
	public:
		SPA(const std::span<CharacteristicGraph>& resource_graphs, const CharacteristicGraph& recipe_graph,
		const BasicActionTheory& global_bat, CompleteTopology& topology, const Limits& lim = Limits(), 
		size_t num_threads = std::thread::hardware_concurrency())
		: resource_graphs(resource_graphs), recipe_graph(recipe_graph),
		global_bat(global_bat), topology(topology), lim(lim), action_cache_(PreExpand(topology, global_bat.objects)),
		num_threads_(num_threads) {
			best_candidate_.total_cost = std::numeric_limits<int32_t>::max();
			best_cost_.store(std::numeric_limits<int32_t>::max());
		}

		void UpdateBest(const Candidate& cand) {
			first_generated_.store(true);
			best_cost_.store(cand.total_cost);
			std::lock_guard<std::mutex> lock(best_mutex_);
			if (cand.total_cost < best_candidate_.total_cost) {
				best_candidate_ = cand;
			}
		}

		std::vector<Candidate> Advance(Candidate& cand) {
			std::vector<Candidate> ret;
			Stage prior_stage = std::move(cand.stages.front());
			cand.stages.pop();
			if (!WithinLimits(cand, prior_stage, lim)) {
				return ret;
			}
			const auto& target_ca = prior_stage.recipe_transition->label().act;
			SCS_TRACE(std::this_thread::get_id());

			for (const auto& trans : topology.at(*prior_stage.resource_states).transitions()) {
				for (const auto& concrete_ca : action_cache_.Get(trans.label().act)) {
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

					auto next_state = AddControllerTransition(next_cand, next_stage, { concrete_ca, trans.label().condition }, prior_stage);
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
									UpdateBest(next_cand);
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
			pq_.clear();
			bool first_generated = false;
			Candidate initial_candidate = CreateInitialCandidate(global_bat, resource_graphs, topology, recipe_graph);
			pq_.push(initial_candidate);

			std::vector<std::thread> threads;
			threads.reserve(num_threads_);
			for (size_t i = 0; i < num_threads_; ++i) {
				threads.emplace_back(&SPA::ThreadSearch, this);
			}
			for (auto& t : threads) {
				SCS_INFO("Joining thread");
				t.join();
			}

			if (best_candidate_.total_cost != std::numeric_limits<int32_t>::max()) {
				SCS_INFO("Best controller, cost = {}, num transitions = {}", best_candidate_.total_cost, best_candidate_.total_transitions);
				return best_candidate_;
			} else {
				SCS_INFO("Was unable to find any controller for the recipe and resources provided");
				return std::nullopt;
			}
		}

		void ThreadSearch() {
			Candidate cand;
			bool can_terminate = false;
			while (!can_terminate) {
				if (pq_.try_pop(cand)) {
					if (cand.total_cost > best_cost_.load() && first_generated_.load()) {
						can_terminate = true;
						break;
					}

					auto next = Advance(cand);
					for (const auto& c : next) {
						pq_.push(c);
					}
				}
			}
			SCS_INFO("Finished thread {}", std::this_thread::get_id());
		}

	};


}

