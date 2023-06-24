﻿#pragma once

#include <string>

#include "scs/FirstOrderLogic/fol.h"
#include "lts/writers/ofstream.h"

namespace scs {

	struct CgState {
		size_t n;
		Formula final_condition;

		CgState() {}
		CgState(size_t s) : n(s), final_condition(true) {}
		CgState(size_t s, const Formula& final_cond) : n(s), final_condition(final_cond) {}

		bool operator==(const CgState& other) const {
			return (n == other.n) && (final_condition == other.final_condition);
		}
	};


	inline std::ostream& operator<<(std::ostream& os, const CgState& cg) {
		os << std::string("⟨");
		os << std::to_string(cg.n);
		os << "⟩";
		return os;
	}	

}

namespace nightly {

	template <>
	inline void WriteXLabel(const scs::CgState& state, std::ostream& os) {
		os << state.final_condition;
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