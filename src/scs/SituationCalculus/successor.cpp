#include "scs/SituationCalculus/successor.h"
#include "scs/SituationCalculus/situation.h"
#include "scs/SituationCalculus/action.h"
#include "scs/SituationCalculus/successor_extractor.h"

#include "scs/FirstOrderLogic/fol.h"

namespace scs {


	Successor::Successor(const std::vector<Term>& terms, const Formula& f, bool is_local_effect)
		: terms_(terms), formula_(f), is_local_effect_(is_local_effect) {
		ComputeInvolvedActions();
	}
	
	Successor::Successor(std::vector<Term>&& terms, Formula&& f, bool is_local_effect) 
		: terms_(std::move(terms)), formula_(std::move(f)), is_local_effect_(is_local_effect) {
		ComputeInvolvedActions();
	}

	bool Successor::IsLocalEffect() const {
		return is_local_effect_;
	}
	
	const Formula& Successor::Form() const {
		return formula_;
	}

	const std::vector<Term>& Successor::Terms() const {
		return terms_;
	}

	void Successor::ComputeInvolvedActions() {
		SuccessorActionExtractor extractor;
		std::visit(extractor, this->formula_);
		involved_actions_ = extractor.Actions();
	}

	bool Successor::Involves(const Action& a) const {
		return involved_actions_.contains(a.name);
	}
	
	bool Successor::Involves(const CompoundAction& ca) const {
		for (const auto& act : ca.Actions()) {
			if (involved_actions_.contains(act.name)) {
				return true;
			}
		}
		return false;
	}

	/** SSA - Successor State Axiom. Utility function for evaluation by passing fluent current value and situation domain.
	 * Note that a SSA doesn't track values for each fluent, this is a helper func.
	 * 
	 * One per fluent, incorporates positive and negative effects
	 * The given fluent is true if and only if:
	 **  - The action taken would make it true
	 **  - Or it was already true, and performing the action does not make this formula false
	 *
	 */
	bool Successor::Evaluate(bool current_value, const Situation& s, const CoopMatrix* coop_mx, 
	const RoutesMatrix* routes_mx, const Action& action_term, FirstOrderAssignment& assignment) const {
		assignment.Set(scs::Variable{ "a" }, action_term); // @Assumption: the variable for deciding which action is being executed is reserved as "a"
		assignment.Set(scs::Variable{"cv"}, current_value); // @Assumption: set name for current value

		scs::Evaluator eval{ {s, *coop_mx, *routes_mx}, assignment};
		return std::visit(eval, formula_);
	}

}