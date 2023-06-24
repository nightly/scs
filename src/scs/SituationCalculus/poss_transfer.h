#pragma once

#include <vector>

#include "scs/SituationCalculus/action.h"
#include "scs/FirstOrderLogic/object.h"

namespace scs {


	// Matching between In and Out stuff (incomplete)

	static bool FindOut(const CompoundAction& ca, scs::Object n) {
		for (const auto& act : ca.Actions()) {
			if (act.name == "Out") {
				auto val = std::get<scs::Object>(act.terms[1]);
				if (n == val) {
					return true;
				}
			}
		}
		return false;
	}

	bool MatchTransfer(const CompoundAction& ca) {
		// Ensures all transfer actions are matched (In and Out, including the correct resource numbers matching up)
		for (const auto& act : ca.Actions()) {
			if (act.name == "In") {
				if (!FindOut(ca, std::get<Object>(act.terms[1]))) {
					return false;
				}
			}
		}
		return true;
	}


}