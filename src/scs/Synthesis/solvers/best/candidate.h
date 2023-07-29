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
		Situation sit;
		const TransitionType* recipe_transition;
		std::vector<CgState> resource_states;

		size_t plan_state = 0;
		int64_t local_num = 0;
		std::unordered_map<UUID, size_t> explored;
	};

	struct Candidate {
		Plan plan; // lts
		int64_t num = 0;
		int64_t completed_recipe_transitions = 0;

		std::queue<Stage> stages;
		StateCounter counter;
	};

	// Min order comparator for Candidates
	struct CandidateComparator {
	public:
		CandidateComparator() {}

		bool operator () (const Candidate& a, const Candidate& b) {
			return (a.num - a.completed_recipe_transitions) > (b.num - b.completed_recipe_transitions);
		}
	};

}