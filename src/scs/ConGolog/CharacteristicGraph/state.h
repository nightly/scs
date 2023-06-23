#pragma once

#include <string>

namespace scs {

	struct CgState {
		size_t n;

		CgState() {}
		CgState(size_t s) : n(s) {}

		bool operator==(const CgState& other) const {
			return n == other.n;
		}
	};


	inline std::ostream& operator<<(std::ostream& os, const CgState& cg) {
		os << std::to_string(cg.n);
		return os;
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