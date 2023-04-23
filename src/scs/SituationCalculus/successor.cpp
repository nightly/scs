#include "scs/SituationCalculus/successor.h"
#include "scs/SituationCalculus/situation.h"
#include "scs/SituationCalculus/action.h"

#include "scs/FirstOrderLogic/fol.h"

namespace scs {

	Successor::Successor(const RelationalFluent& fluent, bool is_local_effect, const Formula& f)
		: fluent_(&fluent), is_local_effect_(is_local_effect), formula_(f) {}


	const RelationalFluent& Successor::fluent() const {
		return *fluent_;
	}

	bool Successor::IsLocalEffect() const {
		return is_local_effect_;
	}
	
	const Formula& Successor::Form() const {
		return formula_;
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
	bool Successor::Evaluate(const RelationalFluent& rf, const Action& a,  Situation& s) {
		FirstOrderAssignment assignment;
		assignment.Set(scs::Variable{ "a" }, a); // @Assumption: the variable for deciding which action is being executed is reserved as "a"
		// @Todo: other assignment variables from the action type itself? Also, for old value from relational fluent, needs also assignments.
		
		scs::Evaluator eval{ s, assignment};
		return std::visit(eval, formula_);
	}
}