#pragma once

#include "execution_type.h"

namespace scs::examples {

	void RunHingeExtended(const ExecutionType& exec = ExecutionType::AStar, size_t recipe_variant = 3, bool just_export = false);

}
