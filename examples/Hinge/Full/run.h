#pragma once

#include "execution_type.h"

namespace scs::examples {

	void RunHinge(const ExecutionType& exec = ExecutionType::AStar, size_t recipe_variant = 2, bool just_export = false);

}
