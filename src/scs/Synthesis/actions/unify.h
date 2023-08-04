#pragma once

#include "scs/SituationCalculus/action.h"
#include "scs/SituationCalculus/compound_action.h"

namespace scs {

	bool UnifyActions(const Action& facility_act, const Action& recipe_act);
	bool UnifyActions(const CompoundAction& facility_compound_act, const CompoundAction& recipe_compound_act);

	bool UnifyAny(const Action& facility_act, const CompoundAction& recipe_compound_act);
}