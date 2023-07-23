#pragma once

#include <vector>

#include "scs/SituationCalculus/action.h"
#include "scs/FirstOrderLogic/object.h"
#include "scs/SituationCalculus/situation.h"
#include "scs/SituationCalculus/bat.h"

namespace scs {

	static bool FindOut(const CompoundAction& ca, const Action& InAct, size_t i, const RoutesMatrix& rm) {
		for (size_t j = 0; j < ca.Actions().size(); ++j) {
			const auto& act = ca.Actions().at(j);
			if (act.name == "Out") {
				const auto& part = std::get<Object>(act.terms[0]);
				if (part.name() != std::get<Object>(InAct.terms[0]).name()) {
					return false;
				}
				if (rm.Lookup(i, j)) {
					return true;
				}
			}
		}
	}

	inline bool Situation::PossibleTransfer(const CompoundAction& ca, const BasicActionTheory& bat) const {
		if (bat.RoutesMx().IsEmpty()) {
			SCS_CRITICAL("In and Out actions found in resources but Routes Matrix is empty.");
			return false;
		}
		std::unordered_set<Object> parts;

		for (size_t i = 0; i < ca.Actions().size(); ++i) {
			const auto& act = ca.Actions().at(i);
			if (act.name == "In") {
				if (parts.contains(std::get<Object>(act.terms[0]))) {
					return false;
				}
				parts.emplace(std::get<Object>(act.terms[0]));
				bool possible_out = FindOut(ca, act, i, bat.RoutesMx());
				if (!possible_out) {
					return false;
				}
			} else if (act.name == "Out") {
				continue; // already handled by In
			} else {
				bool local = this->Possible(act, bat);
				if (!local) {
					return false;
				}
			}
		}
		return true;
	}

}