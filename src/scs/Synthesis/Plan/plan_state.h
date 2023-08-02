#pragma once

#include <string>

#include "scs/FirstOrderLogic/fol.h"
#include "lts/writers/ofstream.h"

namespace scs {

	struct PlanState {
		size_t n;

		PlanState() {}
		PlanState(size_t s) : n(s) {}

		bool operator==(const PlanState& other) const {
			return (n == other.n);
		}
	};


	inline std::ostream& operator<<(std::ostream& os, const PlanState& state) {
		os << std::to_string(state.n);
		return os;
	}

	inline std::ofstream& operator<<(std::ofstream& os, const PlanState& state) {
		os << std::to_string(state.n);
		return os;
	}

}

namespace nightly {

	template <>
	inline void WriteXLabel(const scs::PlanState& state, std::ostream& os) {
		os << "<" << ">";
	}

}

namespace std {

	template<>
	struct hash<scs::PlanState> {
		size_t operator()(const scs::PlanState& state) const {
			return state.n;
		}
	};

}