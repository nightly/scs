#pragma once

#include <string>

#include "scs/FirstOrderLogic/fol.h"
#include "lts/writers/ofstream.h"

namespace scs {

	struct RecipeState {
		size_t n;

		RecipeState() {}
		RecipeState(size_t s) : n(s) {}

		bool operator==(const RecipeState& other) const {
			return (n == other.n);
		}
	};


	inline std::ostream& operator<<(std::ostream& os, const RecipeState& state) {
		os << std::to_string(state.n);
		return os;
	}

	inline std::ofstream& operator<<(std::ofstream& os, const RecipeState& state) {
		os << std::to_string(state.n);
		return os;
	}

}

namespace nightly {

	//template <>
	//inline void WriteXLabel(const scs::CgState& state, std::ostream& os) {
	//	os << state.final_condition;
	//}

}

namespace std {

	template<>
	struct hash<scs::CgState> {
		size_t operator()(const scs::CgState& state) const {
			return state.n;
		}
	};

}