#pragma once

#include "scs/SituationCalculus/action.h"
#include "scs/SituationCalculus/compound_action.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/Common/strings.h"

namespace scs {

	// f(n) for A* algorithm
	struct Valuation {
		size_t cost = 0; // g(n)
		size_t estimate = 0; // h(n)


		// @param advanced_recipe: if the simple action/compound action has been met of a recipe transition before doing this
		void Update(const CgTransition& transition, bool advanced_recipe) {
			size_t compound_cost = CalculateCompound(transition.act);
			cost += compound_cost;

			// Estimate = cost (total number of compound actions +
			// 1*number of constituent simple actions that are non-Nop) - (completed recipe advancements * 1)
			estimate += compound_cost;
			if (advanced_recipe) {
				estimate -= 1;
			}
		}

		static size_t CalculateCompound(const CompoundAction& ca) {
			size_t ret = 0;
			// Add 1 for each compound action
			ret += 1;

			// For each non-Nop simple action, add 1
			for (const auto& act : ca.Actions()) {
				if (scs::Trim(scs::ToLower(act.name)) != "nop") {
					ret++;
				}
			}
			return ret;
		}

	};

}