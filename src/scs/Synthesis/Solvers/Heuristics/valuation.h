#pragma once

#include "scs/SituationCalculus/action.h"
#include "scs/SituationCalculus/compound_action.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/Common/strings.h"

#include "scs/Synthesis/Solvers/Core/candidate.h"

namespace scs {

	inline void UpdateCost(Candidate& next_candidate, Stage& next_stage, const CompoundAction& ca) {
		next_stage.local_transitions++;
		next_candidate.total_transitions++;

		// Add 1 for each compound action (counterracts pure Nop actions)
		next_candidate.total_cost++;

		// For each non-Nop simple action, add 1
		for (const auto& act : ca.Actions()) {
			if (scs::Trim(scs::ToLower(act.name)) != "nop") {
				next_candidate.total_cost++;
				next_stage.local_cost++;
			}
		}
	
	}

}