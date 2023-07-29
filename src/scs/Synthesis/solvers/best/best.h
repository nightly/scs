#pragma once

#include <optional>
#include <span>
#include <limits>

#include "scs/Synthesis/plan.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/SituationCalculus/bat.h"
#include "scs/Synthesis/Actions/unify.h"
#include "scs/Synthesis/valuation.h"
#include "scs/Synthesis/solvers/best/limits.h"
#include "scs/Synthesis/solvers/best/candidate.h"
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

		CompoundActionCache action_cache;
		Candidate best_candidate;
	public:
		Best(const std::span<CharacteristicGraph>& resource_graphs, const CharacteristicGraph& recipe_graph,
		const BasicActionTheory& global_bat, ITopology& topology, const Limits& lim = Limits()) 
		: resource_graphs(resource_graphs), recipe_graph(recipe_graph),
		 global_bat(global_bat), topology(topology), lim(lim), action_cache(global_bat.objects) {
			best_candidate.num = std::numeric_limits<size_t>::max();
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
			if (cand.num < best_candidate.num) {
				best_candidate = cand;
			}
		}

		bool WithinLimits(const Candidate& cand, const Stage& stage) const {
			if (stage.local_num > lim.stage_limit) {
				return false;
			}
			if (cand.num > lim.global_limit) {
				return false;
			}
			return true;
		}

		void NextStages(Candidate& next_candidate, const Stage& old_stage) {
			next_candidate.completed_recipe_transitions++;
			for (const auto& recipe_trans : recipe_graph.lts.at(old_stage.recipe_transition->to()).transitions()) {
				Stage future_stage;
				// @Todo: only add stages where the transition condition is satasified from the current state
				future_stage.recipe_transition = &recipe_trans;
				future_stage.sit = old_stage.sit;
				future_stage.resource_states = old_stage.resource_states;
				future_stage.local_num = 0;

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

					if (next_stage.sit.Possible(concrete_ca, global_bat)) { // @Todo: also check the trans().label().cond here!
						next_stage.sit = next_stage.sit.Do(concrete_ca, global_bat);
					} else {
						continue;
					}
					SCS_TRACE(fmt::format(fmt::fg(fmt::color::golden_rod),
						"Adding action {}", concrete_ca));

					auto next_state = AddControllerTransition(next_cand, next_stage, {concrete_ca, trans.label().condition}, prior_stage);
					next_stage.resource_states = trans.to();

					SCS_INFO(fmt::format(fmt::fg(fmt::color::cyan),
						"Action {} vs {}", concrete_ca, prior_stage.recipe_transition->label().act));

					if (UnifyActions(prior_stage.recipe_transition->label().act, concrete_ca)) {
						// Facility has completed recipe action
						SCS_INFO(fmt::format(fmt::fg(fmt::color::gold),
							"Found facility action {}", concrete_ca));
						if (recipe_graph.lts.at(prior_stage.recipe_transition->to()).transitions().empty()) {
							// No transitions in next state
							// @Incomplete: check if condition holds in such transitions not just final = true

							// SCS_CRITICAL(prior_stage.recipe_transition->to().n);
							// SCS_CRITICAL(prior_stage.recipe_transition->to().final_condition);
							if (prior_stage.recipe_transition->to().final_condition == scs::Formula{true}) {
								if (next_cand.stages.empty()) {
									// No more stages left in candidate other than this,
									// the next recipe transition has no further transitions
									// i.e. the candidate could be complete 
									UpdateBest(next_cand, first_generated);
									continue;
								} else {
									NextStages(next_cand, next_stage);
									ret.emplace_back(next_cand);
									continue;
								}
							} else {
								// Entering a state which is not final but has no transitions
								SCS_CRITICAL("FNT");
							}
						} else { // Next recipe state has transitions to do
							NextStages(next_cand, next_stage);
							ret.emplace_back(next_cand);
							continue;
						}
					} else { // Not unified recipe action
						next_cand.num++;
						next_stage.local_num++; // @Todo: use calculated cost here instead
					
						next_cand.stages.push(next_stage);
						ret.emplace_back(std::move(next_cand));
					}
				}
			}

			return ret;
		}

		std::optional<Plan> Synthethise() {
			bool first_generated = false;
			std::priority_queue<Candidate, std::vector<Candidate>, CandidateComparator> pq;

			Candidate initial_candidate = CreateInitialCandidate();
			pq.push(initial_candidate);

			while (!pq.empty() && (best_candidate.num >= pq.top().num || !first_generated)) {
				Candidate cand = std::move(pq.top());
				pq.pop();

				auto next = Advance(cand, first_generated);
				for (const auto& c : next) {
					pq.push(c);
				}
			}
			ExportController(std::optional<Plan>{best_candidate.plan});
			return best_candidate.plan;
		}
	
	};
	

}

// @Todo:
// - check recipe transition conditions better emplacing

