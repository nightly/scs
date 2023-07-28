#pragma once

#include <ostream>
#include <vector>

#include "scs/SituationCalculus/compound_action.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/ConGolog/CharacteristicGraph/transition.h"

namespace scs {

	struct RecipeTransition {
	public:
		CompoundAction act;
		Formula condition;
	public:
		RecipeTransition() {}
		RecipeTransition(const CgTransition& trans) : act(trans.act), condition(trans.condition) {}
		RecipeTransition(const CompoundAction& ca, const Formula& cond) : act(ca), condition(cond) {}

		bool operator==(const RecipeTransition& other) const {
			return (act == other.act) && (condition == other.condition);
		}
	};

	inline std::ostream& operator<<(std::ostream& os, const RecipeTransition& transition) {
		os << std::string("⟨");
		os << transition.act;
		os << ", ";
		os << transition.condition;
		os << "⟩";
		return os;
	}

}