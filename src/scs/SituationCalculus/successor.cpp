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

	void Successor::ComputeInvolvedActions() {
		SuccessorActionExtractor extractor;
		std::visit(extractor, this->formula_);
		involved_actions_ = extractor.Actions();
	}

	bool Successor::Involves(const Action& a) const {
		return involved_actions_.contains(a.name);
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
	bool Successor::Evaluate(bool current_value, const Action& action_term, const Situation& s) const {
		FirstOrderAssignment assignment;
		assignment.Set(scs::Variable{ "a" }, action_term); // @Assumption: the variable for deciding which action is being executed is reserved as "a"

		// Sub in SSA terms using action term
		for (size_t i = 0; i < terms_.size(); ++i) {
			const auto& obj = std::get<Object>(action_term.parameters[i]); // Performing an action, must have complete object literals
			if (const scs::Variable* var_ptr = std::get_if<Variable>(&terms_.at(i))) {
				assignment.Set(*var_ptr, obj);
			}
		}

		scs::Evaluator eval{ s, assignment};
		return std::visit(eval, Formula(BinaryConnective(formula_, current_value, BinaryKind::Disjunction)));
	}

	// @Todo: technically Evaluate should take a FOL Domain, not a Situation, but it's unlikely to matter in eval for successors

}

/**
* @Notes:
* The FOL assignment is based on the action_type
*
* 
**/