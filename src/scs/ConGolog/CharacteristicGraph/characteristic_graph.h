#pragma once

#include "scs/LTS/src/lts/lts.h"
#include "scs/ConGolog/CharacteristicGraph/state.h"
#include "scs/ConGolog/CharacteristicGraph/transition.h"

namespace scs {


	class CharacteristicGraph {
	public:
		nightly::LTS<size_t, Transition> lts;

	public:
		CharacteristicGraph() {
			lts.set_initial_state(0);
		}
	};


}