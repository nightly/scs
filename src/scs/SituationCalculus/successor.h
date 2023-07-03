#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <ostream>
#include <optional>

#include "scs/SituationCalculus/relational_fluent.h"
#include "scs/FirstOrderLogic/formula.h"
#include "scs/SituationCalculus/situation.h"
#include "scs/SituationCalculus/action.h"
#include "scs/SituationCalculus/coop_matrix.h"
#include "scs/SituationCalculus/routes_matrix.h"
#include "scs/FirstOrderLogic/assignment.h"

namespace scs {

	/** SSA - Successor State Axiom
	**/
	struct Successor {
	private:
		std::vector<Term> terms_;
		Formula formula_;

		std::unordered_set<std::string> involved_actions_;
		bool is_local_effect_ = true;
	public:
		Successor() = default;
		Successor(const std::vector<Term>& terms, const Formula& f, bool is_local_effect = false);
		Successor(std::vector<Term>&& terms, Formula&& f, bool is_local_effect = false);
		
		bool IsLocalEffect() const;
		const Formula& Form() const;
		const std::vector<Term>& Terms() const;
		const auto& InvolvedActions() const { return involved_actions_; }
		bool Involves(const Action& a) const;
		bool Involves(const CompoundAction& a) const;

		void ComputeInvolvedActions();
		bool Evaluate(bool current_value, const Situation& s, const CoopMatrix* coop_mx, const RoutesMatrix* routes_mx,
			const Action& action_term, FirstOrderAssignment& assignment) const;
		bool Evaluate(bool current_value, const Situation& s, const CoopMatrix* coop_mx, const RoutesMatrix* routes_mx,
			const CompoundAction& ca_term, FirstOrderAssignment& assignment) const;

	};
}
