#pragma once

namespace scs {

	struct Limits {
		size_t stage_transition_limit = 3; // For a given recipe transition, number of compound actions before disregarding
		size_t stage_cost_limit = 50;

		size_t global_transition_limit = 10; // For the entire controller, maximum number of compound actions
		size_t global_cost_limit = 200; // Cost = number of compound actions + cost of individual actions

		size_t fairness_limit = 10; // Cyclicality limit
	};

}