#pragma once

#include <vector>

#include "scs/FirstOrderLogic/object.h"
#include "scs/ConGolog/CharacteristicGraph/transition.h"

namespace scs {

	std::vector<CgTransition> TransitionsFromPermutations(const std::vector<std::vector<scs::Object>>& perm, const CgTransition& original);

}