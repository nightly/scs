#pragma once

#include "scs/ConGolog/CharacteristicGraph/state.h"
#include "scs/ConGolog/CharacteristicGraph/transition.h"
#include "scs/LTS/src/lts/lts.h"

namespace scs {


	class CharacteristicGraph {
	public:
		nightly::LTS<CgState, CgTransition> lts;

	public:
		CharacteristicGraph() {
			lts.set_initial_state(0);
		}
	};


}

#include "scs/ConGolog/CharacteristicGraph/export.h"