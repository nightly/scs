#pragma once

#include <queue>
#include <cstdint>

#include "scs/Synthesis/Solvers/Core/stage.h"
#include "scs/Synthesis/Plan/plan.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/ConGolog/CharacteristicGraph/utils/utils.h"
#include "scs/Common/uuid.h"

#include <ankerl/unordered_dense.h>

namespace scs {

	struct Candidate {
		Plan plan; // lts
		int32_t total_transitions = 0;
		int32_t total_cost = 0;
		int32_t completed_recipe_transitions = 0;
		size_t plan_lts_state = 0;

		ankerl::unordered_dense::map<UUID, uint16_t> explored;
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