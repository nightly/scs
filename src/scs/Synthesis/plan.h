#pragma once

#include <vector>

#include "scs/SituationCalculus/compound_action.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/Synthesis/recipe_transition.h"
#include "scs/Synthesis/recipe_state.h"

namespace scs {

	struct Plan {
		nightly::LTS<RecipeState, RecipeTransition> lts;
	};

}