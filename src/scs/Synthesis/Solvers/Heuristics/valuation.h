#pragma once

#include "scs/SituationCalculus/action.h"
#include "scs/SituationCalculus/compound_action.h"
#include "scs/SituationCalculus/bat.h"

#include "scs/Synthesis/Solvers/Core/candidate.h"

namespace scs {

	void UpdateCost(Candidate& next_candidate, Stage& next_stage, const BasicActionTheory& bat,
		const CompoundAction& ca, const CompoundAction& target_ca);

}