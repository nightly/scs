#pragma once

#include <vector>

#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"

namespace scs {
	using TopologyState = std::vector<CgState>;
	using TopologyTransition = std::vector<CgTransition>;
}
