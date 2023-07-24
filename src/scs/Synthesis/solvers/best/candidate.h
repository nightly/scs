#pragma once

#include <unordered_map>

#include "scs/Synthesis/plan.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/ConGolog/CharacteristicGraph/utils/utils.h"
#include "scs/Common/uuid.h"

namespace scs {

	using TransitionType = nightly::Transition<CgState, CgTransition>;

	struct Stage {
		Situation s;
		const TransitionType* recipe_transition;
		std::vector<CgState> resource_states;

		size_t local_num;
	};

	struct Candidate {
		Plan plan;
		size_t num;

		std::queue<Stage> stages;
		StateCounter counter;
		std::unordered_map<uint64_t, size_t> visited;
	};

	// Min order comparator for Candidates
	struct CandidateComparator {
	public:
		CandidateComparator() {}

		bool operator () (const Candidate& a, const Candidate& b) {
			return a.num > b.num;
		}
	};

}