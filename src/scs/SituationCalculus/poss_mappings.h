#pragma once

#include <vector>

#include "scs/SituationCalculus/action.h"
#include "scs/FirstOrderLogic/object.h"
#include "scs/SituationCalculus/situation.h"
#include "scs/SituationCalculus/bat.h"

#include <ankerl/unordered_dense.h>

namespace scs {

	bool PossibleTransfer(const Situation& s, const CompoundAction& ca, const BasicActionTheory& bat);
	static bool FindOut(const CompoundAction& ca, const Action& InAct, size_t i, const BasicActionTheory& bat, const Situation& s);

	bool PossibleRadial(const Situation& s, const CompoundAction& ca, const BasicActionTheory& bat);

}