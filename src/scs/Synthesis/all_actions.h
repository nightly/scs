#pragma once

#include <vector>

#include "scs/FirstOrderLogic/fol.h"

namespace scs {

	std::vector<Action> ExtractActions(const Action& action_type, const Situation& s) {
		std::vector<scs::Action> ret;
		for (const auto& term : action_type.parameters) {
			if (auto* p = std::get_if<Variable>(&term)) {
				for (const auto& obj : s.objects) {

				}
			}
		}


		return ret;
	}

}