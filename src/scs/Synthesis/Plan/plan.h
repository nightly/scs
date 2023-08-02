#pragma once

#include <vector>

#include "scs/SituationCalculus/compound_action.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/Synthesis/plan/plan_transition.h"
#include "scs/Synthesis/plan/plan_state.h"

namespace scs {

	struct Plan {
		nightly::LTS<PlanState, PlanTransition> lts;
	};

}