#pragma once

#include <string>
#include <vector>

#include "scs/FirstOrderLogic/fol.h"
#include "lts/writers/ofstream.h"

namespace scs {

	struct CgState {
		size_t n = 0;
		Formula final_condition = false;
		std::vector<Variable> live_variables;

		CgState() {}
		CgState(size_t s) : n(s) {}
		CgState(size_t s, const Formula& final_cond) : n(s), final_condition(final_cond) {}
		CgState(size_t s, const Formula& final_cond, std::vector<Variable> live)
			: n(s), final_condition(final_cond), live_variables(std::move(live)) {}

		bool operator==(const CgState& other) const {
			// We ignore the final condition for search `at()` convenience, as state numbers should be unique enough.
			return (n == other.n);
		}

		bool StrongEquality(const CgState& other) const {
			return (n == other.n) && (final_condition == other.final_condition)
				&& (live_variables == other.live_variables);
		}
	};

	inline std::ostream& operator<<(std::ostream& os, const CgState& state) {
		os << std::string("⟨");
		os << std::to_string(state.n);
		os << ", ";
		os << state.final_condition;
		if (!state.live_variables.empty()) {
			os << ", live={";
			for (size_t i = 0; i < state.live_variables.size(); ++i) {
				os << state.live_variables[i];
				if (i + 1 != state.live_variables.size()) {
					os << ",";
				}
			}
			os << "}";
		}
		os << "⟩";
		return os;
	}	

	inline std::ostream& operator<<(std::ostream& os, const std::vector<CgState>& set) {
		os << "{";
		for (size_t i = 0; i < set.size(); ++i) {
			os << set[i];
			if (i != set.size() - 1) {
				os << ", ";
			}
		}
		os << "}";
		return os;
	}

	inline std::ofstream& operator<<(std::ofstream& os, const CgState& state) {
		os << std::to_string(state.n);
		return os;
	}

}

namespace nightly {

	template <>
	inline void WriteXLabel(const scs::CgState& state, std::ostream& os) {
		os << state.final_condition;
		if (!state.live_variables.empty()) {
			os << " | live: ";
			for (size_t i = 0; i < state.live_variables.size(); ++i) {
				os << state.live_variables[i];
				if (i + 1 != state.live_variables.size()) {
					os << ",";
				}
			}
		}
	}

}

namespace std {
	
	template<>
	struct hash<scs::CgState> {
		size_t operator()(const scs::CgState& state) const {
			return state.n;
		}
	};

}

namespace scs {
	
	inline size_t hash_value(const CgState& state) {
		return std::hash<CgState>()(state);
	}
}
