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

		std::vector<std::string> involved_actions_;
	public:
		Successor() = default;
		Successor(const RelationalFluent& fluent, bool is_local_effect, const Formula& f);
		
		const RelationalFluent& fluent() const;
		bool IsLocalEffect() const;
		const Formula& Form() const;
		const auto& InvolvedActions() const { return involved_actions_; }

		void ComputeInvolvedActions();
		bool Evaluate(bool current_value, const Action& action_term, const Action& action_type, Situation& s);
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