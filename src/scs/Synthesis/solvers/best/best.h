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
	public:
		Best(const std::span<CharacteristicGraph>& resource_graphs, const CharacteristicGraph& recipe_graph,
		const BasicActionTheory& global_bat, ITopology& topology, const Limits& lim = Limits()) 
			: resource_graphs(resource_graphs), recipe_graph(recipe_graph),
			global_bat(global_bat), topology(topology), lim(lim), action_cache(global_bat.Initial().objects) {}

		Candidate CreateInitialCandidate() const {
			Candidate ret;
			const Situation& s0 = global_bat.Initial();
			std::vector<CgState> resources_states(resource_graphs.size(), CgState{0});
			for (const auto& transition : recipe_graph.lts.at(0).transitions()) {
				Stage stage;
				stage.resource_states = resources_states;
				stage.recipe_transition = &transition;
				stage.s = s0;
				stage.local_num = 0;
				ret.stages.emplace(stage);
			}

			return ret;
		}

		std::vector<Stage> Advance(const Stage& previous_stage) {
			std::vector<Stage> ret;

			for (const auto& trans : topology.at(previous_stage.resource_states).transitions()) {
				//for (const auto& ca : action_cache.Get()) {

				//}

			}

			return ret;
		}

		std::optional<Plan> Synthethise() {
			bool first_generated = false;
			std::priority_queue<Candidate, std::vector<Candidate>, CandidateComparator> pq;
			Candidate best_candidate;
			best_candidate.num = std::numeric_limits<size_t>::max();

			Candidate initial_candidate = CreateInitialCandidate();
			pq.push(initial_candidate);

			while (!pq.empty() && (best_candidate.num >= pq.top().num || !first_generated)) {
				Candidate cand = std::move(pq.top());
				pq.pop();
				Stage stage = std::move(cand.stages.front());
				cand.stages.pop();

				auto next = Advance(stage);

			}
			return std::nullopt;
		}
	
	};
	

}