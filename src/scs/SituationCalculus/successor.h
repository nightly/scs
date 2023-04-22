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
	private:
		bool is_local_effect_ = true;
		const RelationalFluent* fluent_;
		Formula formula_;
	public:
		Successor() = default;
		Successor(const RelationalFluent& fluent, bool is_local_effect, const Formula& f);
		
		const RelationalFluent& fluent() const;
		bool IsLocalEffect() const;
		const Formula& Form() const;

		bool Evaluate(const Action& a, Situation& s);
	};

}

namespace std {

	template <>
	struct std::hash<scs::Successor> {
		size_t operator() (const scs::Successor& successor) const {
			return hash<std::string>()(successor.fluent().name());
		}
	};

}