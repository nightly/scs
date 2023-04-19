#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <ostream>

#include "scs/SituationCalculus/relational_fluent.h"
#include "scs/FirstOrderLogic/formula.h"
#include "scs/SituationCalculus/situation.h"
#include "scs/SituationCalculus/action.h"

namespace scs {

	/** SSA - Successor State Axiom
	**/
	struct Successor {
		const RelationalFluent* fluent; // is storing this necessary?
		// <action, pair<formula, terms>
		std::unordered_map<std::string, std::pair<Formula, std::vector<Term>>> formula_map;

		Successor(const RelationalFluent& fluent);

		void AddAction(const std::string& action, const Formula& formula, const std::vector<Term>& terms);
		void AddAction(std::string&& action, Formula&& formula, std::vector<Term>&& terms);

		bool Evaluate(bool current_value, const Action& a, Situation& s);
	};

}

namespace std {

	template <>
	struct std::hash<scs::Successor> {
		size_t operator() (const scs::Successor& successor) const {
			return hash<std::string>()(successor.fluent->name());
		}
	};

}