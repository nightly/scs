#pragma once

#include <vector>

namespace scs {

	struct StateTracker {
	private:
		size_t n = 0;
		std::vector<size_t> current_states;
	public:
		StateTracker() : n(0) {}

		size_t Increment() {
			n++;
			return;
		}

		const std::vector<size_t>& CurrentStates() const {
			return current_states;
		}

	};

}