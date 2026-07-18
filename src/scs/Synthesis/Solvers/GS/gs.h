#pragma once

#include <optional>
#include <span>
#include <random>
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
#include "scs/Synthesis/Solvers/Core/result.h"
#include "scs/Synthesis/Solvers/GS/gs_comparator.h"

#include "scs/Common/timer.h"
#include "scs/Common/print.h"
#include "scs/Combinatorics/Utils/duplicates.h"

#ifdef max
	#undef max
#endif

namespace scs {

	using TransitionType = nightly::Transition<CgState, CgTransition>;

	struct GS {
	public:
		const std::span<CharacteristicGraph>& resource_graphs;
		const CharacteristicGraph& recipe_graph;
		const BasicActionTheory& global_bat;
		const Limits& lim;
		ITopology& topology;
		bool first_generated_ = false;
		bool markovian_situations_ = true;
		
		bool shuffling_ = false;
		std::mt19937 rng_;
		
		Cache cache_;
		Candidate best_candidate_;
		
		size_t visited_situations_ = 0;
		const SearchControl* search_control_ = nullptr;
		bool cancelled_ = false;
	public:
		GS(const std::span<CharacteristicGraph>& resource_graphs, const CharacteristicGraph& recipe_graph,
		const BasicActionTheory& global_bat, ITopology& topology,
		const Limits& lim = Limits(), bool shuffling = true,
		const std::mt19937& rng = std::mt19937(std::random_device{}()), bool markovian_situations = true)
		: resource_graphs(resource_graphs), recipe_graph(recipe_graph),
		global_bat(global_bat), lim(lim), topology(topology),
		markovian_situations_(markovian_situations), shuffling_(shuffling), rng_(rng), cache_(global_bat.objects) {
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
			
			// @Performance: consider transitions_shuffled() 
			for (const auto& trans : topology.at(*current_stage.resource_states).transitions()) {
				for (const auto& concrete_ca : cache_.Get(trans.label().act)) {
					if (search_control_ != nullptr && search_control_->StopRequested()) {
						cancelled_ = true;
						return ret;
					}
					if (concrete_ca.AreAllNop()) {
						continue;
					}
					if (!Legal(concrete_ca, target_ca, global_bat)) {
						continue;
					}
					if (!cache_.Possible(current_stage.sit, concrete_ca, global_bat, markovian_situations_)
						|| !Holds(current_stage, trans.label().condition, global_bat)) {
						continue;
					}
					visited_situations_++;

					Candidate next_cand = cand;
					Stage next_stage = current_stage;
					next_stage.sit = cache_.Progress(next_stage.sit, concrete_ca, global_bat, markovian_situations_);
					next_stage.resource_states = &trans.to();

					AddControllerTransition(next_cand, next_stage, { concrete_ca, trans.label().condition }, current_stage);
					UpdateCost(next_cand, next_stage, global_bat, concrete_ca, target_ca);

					// Facility has completed recipe action
					if (UnifyActions(concrete_ca, target_ca)) {
						next_cand.completed_recipe_transitions++;
						SCS_INFO(fmt::format(fmt::fg(fmt::color::gold),
							"Found facility action {} for {} [{}]", concrete_ca, target_ca, next_cand.completed_recipe_transitions));
						if (!(recipe_graph.lts.at(next_stage.recipe_transition.to()).transitions().empty())) {
							NextStages(next_cand, next_stage, recipe_graph, global_bat, lim, &trans.to(),
								cache_.SimpleExecutor());
							ret.emplace_back(next_cand);
							continue;
						} else {
							if (!Holds(next_stage, next_stage.recipe_transition.to().final_condition, global_bat)) {
								// Since next state has no transitions - it is a Final state and Final must hold for a terminating recipe
								continue;
							}
						}

						if (next_cand.stages.empty()) {
							first_generated_ = true;
							SCS_TRACE("Last sit = \n {}", next_stage.sit);
							SCS_TRACE("Last resources = {}", *next_stage.resource_states);
							const auto previous_cost = best_candidate_.total_cost;
							UpdateBest(next_cand, best_candidate_);
							if (best_candidate_.total_cost < previous_cost && search_control_ != nullptr
								&& search_control_->on_best_candidate) {
								search_control_->on_best_candidate(best_candidate_, Statistics());
							}
							return ret;
						} else {
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

		[[nodiscard]] SynthesisStatistics Statistics() const {
			return SynthesisStatistics{
				.visited_situations = visited_situations_,
				.action_considerations = cache_.SizeComplete(),
				.cached_fluent_states = cache_.SizeSituationStates(),
				.cache_hits = cache_.SituationCacheHits(),
				.topology_states = topology.lts().NumOfStates(),
				.topology_transitions = topology.lts().NumOfTransitions(),
			};
		}

		SynthesisReport Synthesise(const SearchControl& control = {}) {
			visited_situations_ = 0;
			cancelled_ = false;
			search_control_ = &control;
			best_candidate_ = Candidate{};
			best_candidate_.total_cost = std::numeric_limits<int32_t>::max();
			if (shuffling_) {
				for (auto& [p1, p2] : topology.lts().states()) {
					topology.at(p1).transitions_shuffled(rng_);
				}
			}

			first_generated_ = false;
			std::priority_queue<Candidate, std::vector<Candidate>, GreedyCandidateComparator> pq;

			Candidate initial_candidate = CreateInitialCandidate(global_bat, resource_graphs, topology, recipe_graph,
				cache_.SimpleExecutor());
			pq.push(initial_candidate);

			while (!pq.empty() && !first_generated_) {
				if (control.StopRequested()) {
					cancelled_ = true;
					break;
				}
				Candidate cand = std::move(pq.top());
				pq.pop();
				SCS_INFO(fmt::format(fmt::fg(fmt::color::orchid), 
					"[Pop] Completed transitions = {}, total transitions = {}", cand.completed_recipe_transitions, 
					cand.total_transitions));
				
				auto next = Advance(cand);
				for (const auto& c : next) {
					pq.push(c);
				}
			}
			search_control_ = nullptr;
			SynthesisReport report;
			report.statistics = Statistics();
			if (best_candidate_.total_cost != std::numeric_limits<int32_t>::max()) {
				SCS_INFOSTATS("Greedy controller, cost = {}, num transitions = {}", best_candidate_.total_cost, best_candidate_.total_transitions);

				#if (SCS_STATS_OUTPUT == 1)
					SCS_STATS("Number of action considerations = {}", cache_.SizeComplete());
					SCS_STATS("Number of cached fluent states = {}, cache hits = {}",
						cache_.SizeSituationStates(), cache_.SituationCacheHits());
					SCS_STATS("Number of visited situations = {}", visited_situations_);
					SCS_STATS("Number of topology states = {}, number of topology transitions = {}", topology.lts().NumOfStates(), 
						topology.lts().NumOfTransitions());
				#endif
				#if (SCS_MINIMAL_STATS == 1)
					SCS_MINSTATS("Number of visited situations = {}", visited_situations_);
				#endif

				report.status = cancelled_ ? SynthesisStatus::Cancelled : SynthesisStatus::Solved;
				report.candidate = best_candidate_;
				return report;
			} else {
				if (!cancelled_) {
					SCS_CRITICAL("Was unable to find any controller for the recipe and resources provided");
				}
				report.status = cancelled_ ? SynthesisStatus::Cancelled : SynthesisStatus::NoController;
				return report;
			}
		}

		std::optional<Candidate> Synthethise() {
			return Synthesise().candidate;
		}

	};


}
