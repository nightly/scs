#pragma once

#include <string>
#include <filesystem>
#include <vector>
#include <sstream>

#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/ConGolog/Parser/fol.h"
#include "scs/Common/strings.h"

#include "lts/lts.h"
#include "lts/parsers/parsers.h"

namespace scs {

	void ParseCharacteristicGraph(const std::filesystem::path& path);

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
		scs::FolParser parser(final_condition);
		state.final_condition = parser.ParseFormula();

		return state;
	}

	template <>
	CgTransition ParseTransitionString(const std::string& str) {
		CgTransition transition;
		
		std::size_t pi_pos = str.find("π");
		std::size_t curly_braces_start_pos = str.find("{");
		std::size_t curly_braces_end_pos = str.find("},");
		std::size_t angle_brace_end_pos = str.find("⟩");
		if (pi_pos != std::string::npos) {
			std::string all_vars = str.substr(pi_pos + 2, curly_braces_start_pos - pi_pos - 2);
			std::stringstream ss(all_vars);
			std::string var;
			while (std::getline(ss, var, ',')) {
				transition.vars.emplace_back(scs::Trim(var));
			}
		}
		std::string ca = str.substr(curly_braces_start_pos, (curly_braces_end_pos - curly_braces_start_pos) + 1);

		std::string cond_str = str.substr(curly_braces_end_pos + 3, angle_brace_end_pos - (curly_braces_end_pos + 3));
		scs::FolParser parser(cond_str);
		transition.condition = parser.ParseFormula();

		return transition;
	}

}