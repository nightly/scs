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
		global_bat(global_bat), topology(topology), lim(lim), action_cache(global_bat.Initial().objects) {
			best_candidate.num = std::numeric_limits<size_t>::max();
		}

		Candidate CreateInitialCandidate() const {
			Candidate ret;
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

		CgTransition FlattenTransition(const std::vector<CgTransition>& trans, const CompoundAction& ca) const {
			CgTransition ret;
			ret.act = ca;
			for (const auto& t : trans) {
				ret.condition = ret.condition && t.condition;
				ret.vars.insert(ret.vars.end(), t.vars.begin(), t.vars.end());
			}
			return ret;
		}

		size_t AddControllerTransition(Candidate& candidate, const CgTransition& flattened_transition, const Stage& previous_stage) const {
			size_t next = candidate.counter.Increment();
			candidate.plan.lts.AddTransition(previous_stage.plan_state, flattened_transition, next);
			return next;
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
			for (const auto& recipe_trans : recipe_graph.lts.at(old_stage.recipe_transition->to()).transitions()) {
				Stage future_stage;
				future_stage.recipe_transition = &recipe_trans;
				future_stage.sit = old_stage.sit;
				future_stage.resource_states = old_stage.resource_states;
				future_stage.local_num = 0;
				// @Todo: only add stages where the transition condition is satasified from the current state
				next_candidate.stages.emplace(future_stage);
			}
		}

		std::vector<Candidate> Advance(Candidate& cand, bool& first_generated) {
			std::vector<Candidate> ret;
			Stage stage = std::move(cand.stages.front());
			cand.stages.pop();
			if (!WithinLimits(cand, stage)) {
				return ret;
			}
			
			for (const auto& trans : topology.at(stage.resource_states).transitions()) {
				const auto& abstract_ca = action_cache.CompoundActionFromTransition(trans.label());
				for (const auto& concrete_ca : action_cache.Get(abstract_ca)) {
					Candidate next_cand = cand;
					Stage next_stage = stage;
					auto next_transition = FlattenTransition(trans.label(), concrete_ca);
					if (next_stage.sit.Possible(next_transition.act, global_bat)) {
						next_stage.sit = next_stage.sit.Do(next_transition.act, global_bat);
					} else {
						continue;
					}
					auto next_state = AddControllerTransition(next_cand, next_transition, stage);
					next_stage.resource_states = trans.to();
					next_stage.plan_state = next_state;

					if (UnifyActions(stage.recipe_transition->label().act, concrete_ca)) {
						// Facility has completed recipe action
						next_cand.completed_recipe_transitions++;
						if (!(recipe_graph.lts.at(stage.recipe_transition->to()).transitions().empty())) {
							// No transitions in next state
							// @Incomplete: check if condition holds in such transitions not just final = true
							if (stage.recipe_transition->to().final_condition == scs::Formula{true}) {
								if (next_cand.stages.empty()) {
									// No more stages left in candidate, the next recipe transition has no further transitions
									// i.e. the candidate is complete, and 
									UpdateBest(next_cand, first_generated);
									continue;
								} else {
									NextStages(next_cand, next_stage);
									continue;
								}
							}
						} else { // Next recipe state has transitions to do
							NextStages(next_cand, next_stage);
							continue;
						}
					} else { // Not unified recipe action
						next_cand.num++;
						next_stage.local_num++;
					}

					next_cand.stages.push(next_stage);
					ret.emplace_back(std::move(next_cand));
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
			return best_candidate.plan;
		}
	
	};
	

}

// @Todo:
// - check recipe transition conditions better emplacing

