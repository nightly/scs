#pragma once

#include <queue>
#include <vector>
#include <cstdint>

#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/ConGolog/CharacteristicGraph/utils/utils.h"
#include "scs/Common/uuid.h"


namespace scs {

	using TransitionType = nightly::Transition<CgState, CgTransition>;

	enum class StageType {
		Regular = 0,
		Pi = 1,
	};

	struct Stage {
		int32_t local_transitions = 0;
		int32_t local_cost = 0;
		StageType type = StageType::Regular;

		const TransitionType* recipe_transition;
		const std::vector<CgState>* resource_states;
		Situation sit;
	};

}