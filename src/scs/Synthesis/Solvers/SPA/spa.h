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

		CompoundActionCache ca_cache_; // Must be pre-expanded
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
		global_bat(global_bat), topology(topology), lim(lim), ca_cache_(PreExpand(topology, global_bat.objects)),
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
			Stage current_stage = std::move(cand.stages.front());
			cand.stages.pop();
			if (!WithinLimits(cand, current_stage, lim)) {
				if (current_stage.type == StageType::Regular) {
					return ret;
				} else if (current_stage.type == StageType::Pi) {
					ret.emplace_back(cand);
					return ret;
				}
			}
			const auto& target_ca = current_stage.recipe_transition.label().act;
			SCS_INFO(std::this_thread::get_id());

			for (const auto& trans : topology.at(*current_stage.resource_states).transitions()) {
				for (const auto& concrete_ca : ca_cache_.Get(trans.label().act)) {
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

					// Facility has completed recipe action
					if (UnifyActions(concrete_ca, target_ca)) {
						next_cand.completed_recipe_transitions++;
						SCS_INFO(fmt::format(fmt::fg(fmt::color::gold),
							"Found facility action {} for {} [{}]", concrete_ca, target_ca, next_cand.completed_recipe_transitions));

						if (recipe_graph.lts.at(next_stage.recipe_transition.to()).transitions().empty()) {
							// No transitions in next state
							if (Holds(next_stage, next_stage.recipe_transition.to().final_condition, global_bat)) {
								// Final state
								if (next_cand.stages.empty()) {
									// All recipe transitions processed for candidate, potentially update best
									UpdateBest(next_cand);
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
							NextStages(next_cand, next_stage, recipe_graph, global_bat, lim, &trans.to(), 
								ca_cache_.SimpleExecutor());
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
			pq_.clear();
			bool first_generated = false;
			Candidate initial_candidate = CreateInitialCandidate(global_bat, resource_graphs, topology, recipe_graph, 
				ca_cache_.SimpleExecutor());
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

