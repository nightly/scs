#pragma once

#include <string>

#include "characteristic_graph.h"

#include "lts/lts.h"
#include "lts/parsers/parsers.h"

namespace scs {

	void ParseCharacteristicGraph();

}


namespace nightly {

	using namespace scs;

	template <>
	CgState ParseStateString(const std::string& str) {
		CgState state;
		std::string state_name, final_condition;

		size_t open_pos = str.find("(");
		state_name = str.substr(0, open_pos);

		size_t close_pos = str.find(")");
		final_condition = str.substr(open_pos + 1, close_pos - open_pos - 1);
		
		state.n = stoull(state_name);
		// state.final_condition = ParseFormula(final_condition);

		return state;
	}

	template <>
	CgTransition ParseTransitionString(const std::string& str) {
		CgTransition transition;
		

		return transition;
	}

}