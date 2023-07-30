#pragma once

namespace scs {

	struct Limits {
		size_t stage_limit = 7; // For a given recipe transition, number of compound actions before disregarding
		size_t global_limit = 100; // For the entire, maximum number of compound actions

		size_t fairness_limit = 10; // Cyclicality limit
	};

}