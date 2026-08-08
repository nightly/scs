#pragma once

#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "types.h"

#include "lts/lts.h"

namespace scs {


	inline nightly::Transition<TopologyState, TopologyTransition> CreateTransition(const std::vector<nightly::Transition<CgState,
	CgTransition>>& combo) {
		nightly::Transition<TopologyState, TopologyTransition> transition;
		transition.to().resize(combo.size());
		transition.label().components = combo;
		bool any_cond = false;

		for (size_t i = 0; i < combo.size(); ++i) {
			transition.to().at(i) = combo[i].to();
			for (const auto& action : combo[i].label().act.Actions()) {
				transition.label().act.AppendAction(action);
			}
			if (combo[i].label().condition != Formula{true}) {
				// Simplify from having redundant 'true' formulas included in the chain
				if (any_cond) {
					transition.label().condition = transition.label().condition && combo[i].label().condition;
				} else {
					any_cond = true;
					transition.label().condition = combo[i].label().condition;
				}
			}
			transition.label().vars.insert(transition.label().vars.end(), combo[i].label().vars.begin(), combo[i].label().vars.end());
		}

		return transition;
	}

}
