#pragma once

#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "types.h"

#include "lts/lts.h"

namespace scs {


	inline nightly::Transition<scs::CgState, scs::CgTransition> FlagValue() {
		// This should never be really called, as it would mean ending up in a CgState
		// that has no transition for a resource
		nightly::Transition<scs::CgState, scs::CgTransition> flag_value;
		flag_value.label().act = Action{ "ErrorState" };
		return flag_value;
	}

	inline bool IsFlagValue(const nightly::Transition<scs::CgState, scs::CgTransition>& value) {
		return FlagValue() == value;
	}

	inline nightly::Transition<TopologyState, TopologyTransition> CreateTransition(const std::vector<nightly::Transition<CgState,
	CgTransition>>& combo) {
		nightly::Transition<TopologyState, TopologyTransition> transition;
		transition.to().resize(combo.size());
		transition.label().resize(combo.size());

		for (size_t i = 0; i < combo.size(); ++i) {
			transition.to().at(i) = combo[i].to();
			transition.label().at(i) = combo[i].label();
		}

		return transition;
	}

}