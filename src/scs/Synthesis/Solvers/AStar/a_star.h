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
#include "scs/Synthesis/Solvers/Core/result.h"

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
		bool markovian_situations_ = true;

		//std::random_device rd;
		//std::mt19937 rng_{ rd() };

		Cache cache_; // Handles ungrounded actions into grounded actions & stores fluents -> actions

		Candidate best_candidate_;
		
		size_t visited_situations_ = 0;
		const SearchControl* search_control_ = nullptr;
		bool cancelled_ = false;
	public:
		AStar(const std::span<CharacteristicGraph>& resource_graphs, const CharacteristicGraph& recipe_graph,
		const BasicActionTheory& global_bat, ITopology& topology, 
		const Limits& lim = Limits(), bool markovian_situations = true)
		: resource_graphs(resource_graphs), recipe_graph(recipe_graph),
		global_bat(global_bat), lim(lim), topology(topology),
		markovian_situations_(markovian_situations), cache_(global_bat.objects) {
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
				for (const auto& bound_step : InstantiateTopologyTransition(trans, current_stage, global_bat)) {
					if (search_control_ != nullptr && search_control_->StopRequested()) {
						cancelled_ = true;
						return ret;
					}
					const auto& concrete_ca = bound_step.action;
					if (!Legal(concrete_ca, target_ca, global_bat)) {
						continue;
					}
					auto execution_objects = StageSupport(current_stage, global_bat);
					AddGroundActionObjects(execution_objects, concrete_ca);
					if (!cache_.Possible(current_stage.sit, concrete_ca, global_bat,
						markovian_situations_, &execution_objects)) {
						continue;
					}
					visited_situations_++;

					Candidate next_cand = cand;
					Stage next_stage = current_stage;
					next_stage.sit = cache_.Progress(next_stage.sit, concrete_ca, global_bat,
						markovian_situations_, &execution_objects);
					next_stage.resource_states = &trans.to();
					next_stage.resource_bindings = bound_step.resource_bindings;

					AddControllerTransition(next_cand, next_stage, {concrete_ca, trans.label().condition}, current_stage);
					UpdateCost(next_cand, next_stage, global_bat, concrete_ca, target_ca);

					// Facility has completed recipe action
					if (UnifyActions(concrete_ca, target_ca)) {
						next_cand.completed_recipe_transitions++;
						next_stage.recipe_bindings = next_stage.recipe_bindings.Project(
							next_stage.recipe_transition.to().live_variables);
						SCS_INFO(fmt::format(fmt::fg(fmt::color::gold),
							"Found facility action {} for {} [{}]", concrete_ca, target_ca, next_cand.completed_recipe_transitions));

						const auto& recipe_state = recipe_graph.lts.at(next_stage.recipe_transition.to());
						const bool semantically_final = Holds(next_stage,
							next_stage.recipe_transition.to().final_condition, global_bat);
						if (semantically_final && next_cand.stages.empty()) {
							/* All recipe transitions processed for candidate, potentially update best
							No other stages in the queue. We assume recipes must be terminating already.
							Any candidate that failed to simulate a legal transition would be disregarded 
							with WithinLimits check, so every required transition has already been done.
							*/
							first_generated_ = true;
							const auto previous_cost = best_candidate_.total_cost;
							UpdateBest(next_cand, best_candidate_);
							if (best_candidate_.total_cost < previous_cost && search_control_ != nullptr
								&& search_control_->on_best_candidate) {
								search_control_->on_best_candidate(best_candidate_, Statistics());
							}
						}

						if (!recipe_state.transitions().empty()) {
							Candidate continuing = next_cand;
							NextStages(continuing, next_stage, recipe_graph, global_bat, lim, &trans.to());
							if (!continuing.stages.empty()) {
								ret.emplace_back(std::move(continuing));
							}
						} else if (!semantically_final) {
							continue;
						} else if (!next_cand.stages.empty()) {
							ret.emplace_back(std::move(next_cand));
						}
						continue;
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

			first_generated_ = false;
			std::priority_queue<Candidate, std::vector<Candidate>, CandidateComparator> pq;

			Candidate initial_candidate = CreateInitialCandidate(global_bat, resource_graphs, topology, recipe_graph);
			const auto initial_objects = RelevantObjects(global_bat.Initial(), global_bat);
			if (Holds(global_bat.Initial(), recipe_graph.lts.initial_state().final_condition,
				global_bat, {}, initial_objects)) {
				best_candidate_ = initial_candidate;
				first_generated_ = true;
			}
			if (!initial_candidate.stages.empty()) {
				pq.push(initial_candidate);
			}

			while (!pq.empty() && (best_candidate_.total_cost > pq.top().total_cost || !first_generated_)) {
				if (control.StopRequested()) {
					cancelled_ = true;
					break;
				}
				Candidate cand = std::move(pq.top());
				pq.pop();

				auto next = Advance(cand);
				for (const auto& c : next) {
					pq.push(c);
				}
			}
			search_control_ = nullptr;
			SynthesisReport report;
			report.statistics = Statistics();
			if (best_candidate_.total_cost != std::numeric_limits<int32_t>::max()) {
				SCS_INFOSTATS("AStar controller, cost = {}, num transitions = {}", best_candidate_.total_cost, best_candidate_.total_transitions);
				
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
