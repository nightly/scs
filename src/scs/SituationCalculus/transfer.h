#pragma once

#include <vector>

#include "scs/SituationCalculus/action.h"
#include "scs/FirstOrderLogic/object.h"
#include "scs/SituationCalculus/situation.h"

namespace scs {

	inline bool Situation::PossibleTransfer(const CompoundAction& ca, const BasicActionTheory& bat) const {
		std::vector<bool> possible_actions(ca.Actions().size());

		for (const auto& act : ca.Actions()) {
			if (act.name == "In") {
				// Handle here
				bool local;
			} else if (act.name == "Out") {
				// Already handled by In
				continue;
			} else {
				bool local = this->Possible(act, bat);
				if (!local) {
					return false;
				}
			}
		}
		return true;
	}


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

	inline bool MatchTransfer(const CompoundAction& ca) {
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