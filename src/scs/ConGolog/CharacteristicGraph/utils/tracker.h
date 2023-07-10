#pragma once

#include <vector>
#include <optional>

namespace scs {


	struct StateTracker {
	private:
		std::vector<size_t> current_states;
	public:
		StateTracker() : current_states({ 0 }) {}
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

		StateTracker operator+(const StateTracker& other) const {
			StateTracker ret = *this;
			for (const auto& st : other.current_states) {
				ret.AppendState(st);
			}
			return ret;
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

}