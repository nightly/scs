#pragma once

#include <random>

#include "execution_type.h"

namespace scs::examples {

	void RunHingeExtended(const ExecutionType& exec = ExecutionType::AStar, size_t recipe_variant = 3, bool just_export = false,
		bool shuffling = false, const std::mt19937& rng = std::mt19937(std::random_device{}()));

}
