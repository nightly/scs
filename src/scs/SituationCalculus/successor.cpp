#include "scs/SituationCalculus/successor.h"
#include "scs/SituationCalculus/situation.h"
#include "scs/SituationCalculus/action.h"

#include "scs/FirstOrderLogic/fol.h"

namespace scs {

	Successor::Successor(const RelationalFluent& fluent)
		: fluent(&fluent) {}

	void Successor::AddAction(const std::string& action, const Formula& formula, const std::vector<Term>& terms) {
		formula_map[action] = std::make_pair(formula, terms);
	}

	void Successor::AddAction(std::string&& action, Formula&& formula, std::vector<Term>&& terms) {
		formula_map[std::move(action)] = std::make_pair(std::move(formula), std::move(terms));
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
	bool Successor::Evaluate(bool current_value, const Action& a,  Situation& s) {
		scs::Evaluator eval{ s };
		bool ret = current_value; // This could just be given by the fluent pointer

		// Find corresponding action and formula if exists
		if (formula_map.contains(a.name)) {
			FirstOrderAssignment assignment;
			const auto& pair = formula_map[a.name];
			const auto& formula = pair.first;
			const auto& terms = pair.second;

			for (size_t i = 0; i < a.parameters.size(); ++i) {
				const auto& obj = std::get<Object>(a.parameters[i]); // Performing an action, must have complete object literals
	
				if (const scs::Variable* var_ptr = std::get_if<Variable>(&terms[i])) {
					assignment.Set(*var_ptr, obj);
				}
				
			}
			
			Evaluator eval{s, assignment};
			ret = std::visit(eval, formula);
			return ret;
		} else {
			// Otherwise return old fluent value
			return ret;
		}
	}
}

// How to rewrite this:
// First of all, this has to be run for all fluents, so the variables of a fluent must be resubstiuted back in, and the action must also be mapped to the assignment
// So, ideally, the valuations are stored independently