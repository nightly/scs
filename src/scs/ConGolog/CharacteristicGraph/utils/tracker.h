#pragma once

#include <vector>
#include <optional>
#include <ostream>

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

		void AppendUniqueState(size_t el) {
			for (const auto& n : current_states) {
				if (n == el) return;
			}
			current_states.emplace_back(el);
		}

		void RemoveState(size_t el) {
			auto position = std::find(current_states.begin(), current_states.end(), el);
			if (position != current_states.end()) {
				current_states.erase(position);
			}
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

		bool In(size_t el) const {
			for (const auto& n : current_states) {
				if (n == el) return true;
			}
			return false;
		}

	};


	inline std::ostream& operator<< (std::ostream& os, const StateTracker& st) {
		os << "Tracker: ";
		for (size_t i = 0; i < st.CurrentStates().size(); ++i) {
			os << st.CurrentStates().at(i);
			if (i != st.CurrentStates().size() - 1) {
				os << ",";
			}
		}
		return os;
	}

}