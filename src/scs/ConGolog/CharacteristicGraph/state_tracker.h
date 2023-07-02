#pragma once

#include <vector>

namespace scs {

	struct StateCounter {
	private:
		size_t n = 0;
	public:
		StateCounter() : n(0) {}

		size_t Increment() {
			n++;
			return n;
		}
	};

	struct StateTracker {
	private:
		std::vector<size_t> current_states;
	public:
		StateTracker() : current_states({0}) {}
		StateTracker(const std::vector<size_t>& current_states) : current_states(current_states) {}
		StateTracker(std::vector<size_t>&& current_states) : current_states(std::move(current_states)) {}

		const std::vector<size_t>& CurrentStates() const {
			return current_states;
		}

		void Clear() {
			current_states.clear();
		}

		void AppendState(size_t n) {
			current_states.emplace_back(n);
		}

		void SetState(size_t n) {
			Clear();
			AppendState(n);
		}

		void SetStates(std::vector<size_t>&& states) {
			current_states = std::move(states);
		}	
		
		void SetStates(const std::vector<size_t>& states) {
			current_states = states;
		}

	};

	struct StateMeta {
	public:
		int loop_back = -1;
	};

}