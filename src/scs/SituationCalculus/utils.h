#pragma once

#include "scs/FirstOrderLogic/evaluator.h"
#include "scs/SituationCalculus/bat.h"
#include "scs/SituationCalculus/situation.h"

namespace scs {

	bool Holds(const Situation& s, const BasicActionTheory& bat, 
		const Formula& f, const FirstOrderAssignment& assignment = FirstOrderAssignment());


}