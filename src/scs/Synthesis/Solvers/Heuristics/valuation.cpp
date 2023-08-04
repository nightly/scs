#include "valuation.h"

#include "scs/SituationCalculus/action.h"
#include "scs/SituationCalculus/compound_action.h"
#include "scs/SituationCalculus/bat.h"
#include "scs/Synthesis/Solvers/Core/candidate.h"
#include "scs/Synthesis/Actions/unify.h"
#include "scs/Common/strings.h"

namespace scs {

	void UpdateCost(Candidate& next_candidate, Stage& next_stage, const BasicActionTheory& bat, 
	const CompoundAction& ca, const CompoundAction& target_ca) {
		next_stage.local_transitions++;
		next_candidate.total_transitions++;

		// Add 1 for each compound action (counterracts pure Nop actions)
		next_candidate.total_cost++;

		// For each non-Nop simple action, add the cost for each constituent action type as determined by action type
		for (const auto& act : ca.Actions()) {
			assert(bat.types.contains(act.name) && "Basic Action Theory does not have action type for this action");
			bool mandatory_act = false;
			if (UnifyAny(act, target_ca)) {
				mandatory_act = true;
			}

			if (!mandatory_act) {
				next_candidate.total_cost += static_cast<int32_t>(bat.types.at(act.name));
				next_stage.local_cost += static_cast<int32_t>(bat.types.at(act.name));
			}
		}
	}
}