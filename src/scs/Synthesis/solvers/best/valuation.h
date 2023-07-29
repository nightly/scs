#pragma once

#include "scs/SituationCalculus/action.h"
#include "scs/SituationCalculus/compound_action.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/Common/strings.h"

#include "scs/Synthesis/Solvers/Best/candidate.h"

namespace scs {

	inline void UpdateCost(Candidate& next_candidate, Stage& next_stage, const CompoundAction& ca) {
		next_candidate.num++;
		next_stage.local_num++;

		// For each non-Nop simple action, add 1
		for (const auto& act : ca.Actions()) {
			if (scs::Trim(scs::ToLower(act.name)) != "nop") {
				next_candidate.num++;
				next_stage.local_num++;
			}
		}
	
	}

}