#pragma once

#include <vector>

#include "scs/SituationCalculus/action_state.h"

namespace scs {

	enum class DeviationType {
		NonDetBranch = 0,
		Iteration = 1,
		Pick = 2,
	};

	struct Plan {
		std::vector<ActionState> actions;
		std::vector<std::pair<Plan, DeviationType>> branches;
	};

}