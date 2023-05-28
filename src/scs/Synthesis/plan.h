#pragma once

#include <vector>

#include "scs/SituationCalculus/compound_action.h"

namespace scs {

	enum class DeviationType {
		NonDetBranch = 0,
		Iteration = 1,
		Pick = 2,
		InterleavedConc = 3, // Since it can do p first before q or q before p or switch between the two.
	};

	struct Plan {
		std::vector<std::variant<CompoundAction, std::pair<Plan, DeviationType>>> executions;
	};

}