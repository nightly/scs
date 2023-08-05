#include "unify.h"

#include <vector>
#include <unordered_map>

#include "scs/FirstOrderLogic/object.h"
#include "scs/SituationCalculus/action.h"
#include "scs/SituationCalculus/compound_action.h"
#include "scs/Common/log.h"

namespace scs {

	bool UnifyActions(const scs::Action& facility_act, const scs::Action& recipe_act) {
		if (recipe_act.name != facility_act.name) {
			return false;
		}

		// Unify from recipe to facility simple action.
		// Works to the nth recipe term. Any variable included in term n of recipe simple act term can take any value,
		//  object constants must exactly match. Anything past n in recipe parameters will not be considered (= shorthand).
		for (size_t i = 0; i < recipe_act.terms.size(); ++i) {
			if (auto rec_ptr = std::get_if<Object>(&recipe_act.terms[i])) {

				if (auto fac_ptr = std::get_if<Object>(&facility_act.terms[i])) {
					// recipe term i is object, must match exactly
					if (*rec_ptr != *fac_ptr) {
						return false;
					}
				} else if (auto fac_ptr = std::get_if<Variable>(&facility_act.terms[i])) {
					SCS_CRITICAL("Facility action term {} should always be an object not variable", i);
					std::exit(88);
				}


			} else if (auto rec_ptr = std::get_if<Variable>(&recipe_act.terms[i])) {

				if (auto fac_ptr = std::get_if<Object>(&facility_act.terms[i])) {
					continue; // recipe term i is variable, it can take any value
				} else if (auto fac_ptr = std::get_if<Variable>(&facility_act.terms[i])) {
					SCS_CRITICAL("Facility action term {} should always be an object not variable", i);
					std::exit(88);
				}

			}
		}
		return true;
	}

	bool UnifyActions(const scs::CompoundAction& facility_compound_act, const scs::CompoundAction& recipe_compound_act) {
		for (const auto& recipe_act : recipe_compound_act.Actions()) {
			bool unified = false;
			for (const auto& facility_act : facility_compound_act.Actions()) {
				unified = UnifyActions(facility_act, recipe_act);
				if (unified) {
					break;
				}
			}
			if (!unified) {
				return false;
			}
		}
		return true;
	}

	// Given a simple facility action, unify against any recipe action in the target compound action
	// In other words, is the facility_act (a manufacturing action) necessary within the recipe compound action
	bool UnifyAny(const Action& facility_act, const CompoundAction& recipe_compound_act) {
		for (const auto& recipe_act : recipe_compound_act.Actions()) {
			bool unified = false;
			unified = UnifyActions(facility_act, recipe_act);
			if (unified) {
				return true;
			}
		}
		return false;
	}

	// Basically, asserts that any manufacturing ActionTypes are necessary by the recipe, otherwise they are not considered	
	// Other action types (Transfer, Nop, Prepatory) are ignored by this "Legal" check in terms of their necessity
	bool Legal(const CompoundAction& facility_ca, const CompoundAction& recipe_ca, const BasicActionTheory& bat) {
		for (const auto& facility_act : facility_ca.Actions()) {
			if (bat.types.at(facility_act.name) == ActionType::Manufacturing) {
				bool unified = false;
				unified = UnifyActions(facility_act, recipe_ca);
				if (!unified) {
					return false;
				}
			} else {
				continue;
			}
		}
		return true;
	}

}