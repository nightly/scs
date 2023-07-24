#pragma once

#include <vector>

#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"

namespace scs {

	struct Evolution {
		std::vector<CgState> states;
		CompoundAction ca;
	};


	std::size_t hash_value(const nightly::Transition<scs::CgState, scs::CgTransition>& transition) {
		size_t seed = 0;
		boost::hash_combine(seed, std::hash<scs::CgTransition>()(transition.label()));
		boost::hash_combine(seed, std::hash<scs::CgState>()(transition.to()));
		return seed;
	}
}

namespace std {

	template <>
	struct std::hash<nightly::Transition<scs::CgState, scs::CgTransition>> {
		size_t operator() (const nightly::Transition<scs::CgState, scs::CgTransition>& transition) const {
			size_t seed = 0;
			boost::hash_combine(seed, std::hash<scs::CgTransition>()(transition.label()));
			boost::hash_combine(seed, std::hash<scs::CgState>()(transition.to()));
			return seed;
		}
	};

}