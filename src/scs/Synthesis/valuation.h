#pragma once

#include "scs/SituationCalculus/action.h"
#include "scs/SituationCalculus/compound_action.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/Common/strings.h"

namespace scs {

	inline size_t CalculateCompound(const CompoundAction& ca) {
		size_t ret = 0;
		// Add 1 for each compound action
		ret += 1;

		// For each non-Nop simple action, add 1
		for (const auto& act : ca.Actions()) {
			if (scs::Trim(scs::ToLower(act.name)) != "nop") {
				ret++;
			}
		}
		return ret;
	}

}