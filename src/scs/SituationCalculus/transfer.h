#pragma once

#include <vector>

#include "scs/SituationCalculus/action.h"
#include "scs/FirstOrderLogic/object.h"
#include "scs/SituationCalculus/situation.h"
#include "scs/SituationCalculus/bat.h"

#include <ankerl/unordered_dense.h>

namespace scs {

	static bool FindOut(const CompoundAction& ca, const Action& InAct, size_t i, const BasicActionTheory& bat, const Situation& s) {
		for (size_t j = 0; j < ca.Actions().size(); ++j) {
			const auto& act = ca.Actions().at(j);
			if (act.name == "Out") {
				const auto& part = std::get<Object>(act.terms[0]);
				if (part.name() != std::get<Object>(InAct.terms[0]).name()) {
					return false;
				}
				if (bat.RoutesMx().Lookup(i, j) && s.Possible(act, bat)) {
					return true;
				}
			}
		}
		return false;
	}

	inline bool Situation::PossibleTransfer(const CompoundAction& ca, const BasicActionTheory& bat) const {
		if (bat.RoutesMx().IsEmpty()) {
			SCS_CRITICAL("In and Out actions found in resources but Routes Matrix is empty.");
			return false;
		}
		size_t num_in(0), num_out(0);
		std::unordered_map<scs::Object, size_t> parts_count;
		for (const auto& a : ca.Actions()) {
			if (a.name == "In") {
				num_in++;
				parts_count[std::get<Object>(a.terms[0])]++;
			} else if (a.name == "Out") {
				num_out++;
				parts_count[std::get<Object>(a.terms[0])]++;
			}
		}
		if (num_in != num_out) {
			return false;
		}
		for (const auto& [key, val] : parts_count) {
			if (val != 2) {
				return false;
			}
		}

		ankerl::unordered_dense::set<Object> parts;

		for (size_t i = 0; i < ca.Actions().size(); ++i) {
			const auto& act = ca.Actions().at(i);
			if (act.name == "In") {
				if (parts.contains(std::get<Object>(act.terms[0]))) {
					return false;
				}
				parts.emplace(std::get<Object>(act.terms[0]));
				if (!this->Possible(act, bat)) {
					return false;
				}
				bool possible_out = FindOut(ca, act, i, bat, *this);
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