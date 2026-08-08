#pragma once

#include <vector>

#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"

namespace scs {
	using TopologyState = std::vector<CgState>;

	struct TopologyTransition : CgTransition {
		using ComponentTransition = nightly::Transition<CgState, CgTransition>;
		std::vector<ComponentTransition> components;

		bool operator==(const TopologyTransition& other) const {
			return CgTransition::operator==(other) && components == other.components;
		}
	};
}
