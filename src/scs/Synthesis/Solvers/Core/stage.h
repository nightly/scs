#pragma once

#include <queue>
#include <vector>
#include <cstdint>

#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/ConGolog/CharacteristicGraph/utils/utils.h"
#include "scs/Common/uuid.h"


namespace scs {

	using TransitionType = nightly::Transition<CgState, CgTransition>;

	struct Stage {
		const TransitionType* recipe_transition;
		size_t plan_state = 0;
		int32_t local_transitions = 0;
		int32_t local_cost = 0;

		Situation sit;
		const std::vector<CgState>* resource_states;
	};

}