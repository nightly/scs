#pragma once

#include <unordered_map>
#include <queue>
#include <cstdint>

#include "scs/Synthesis/plan.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/ConGolog/CharacteristicGraph/utils/utils.h"
#include "scs/Common/uuid.h"

namespace scs {

	using TransitionType = nightly::Transition<CgState, CgTransition>;

	struct Stage {
		const TransitionType* recipe_transition;
		size_t plan_state = 0;
		int64_t local_transitions = 0;
		int64_t local_cost = 0;

		Situation sit;
		std::vector<CgState> resource_states;
		std::unordered_map<UUID, size_t> explored;
	};

	struct Candidate {
		Plan plan; // lts
		int64_t total_transitions = 0;
		int64_t total_cost = 0;
		int64_t completed_recipe_transitions = 0;

		std::queue<Stage> stages;
		StateCounter counter;
	};

	// Min order comparator for Candidates
	struct CandidateComparator {
	public:
		CandidateComparator() {}

		bool operator () (const Candidate& a, const Candidate& b) {
			return (a.total_cost - a.completed_recipe_transitions) > (b.total_cost - b.completed_recipe_transitions);
		}
	};

}