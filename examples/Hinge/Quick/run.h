#pragma once

#include "execution_type.h"

namespace scs::examples {

	void RunHingeQuick(const ExecutionType& exec = ExecutionType::AStar, size_t recipe_variant = 1, bool just_export = false);

}
