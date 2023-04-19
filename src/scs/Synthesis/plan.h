#pragma once

#include <vector>

#include "scs/SituationCalculus/compound_action.h"

namespace scs {

	enum class DeviationType {
		NonDetBranch = 0,
		Iteration = 1,
		Pick = 2,
	};

	struct Plan {
		std::vector<Action> actions; // Should be compound

		std::vector<std::pair<Plan, DeviationType>> deviations;
	};

}