#pragma once

#include <ostream>
#include <vector>

#include "scs/SituationCalculus/compound_action.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/ConGolog/CharacteristicGraph/transition.h"

namespace scs {

	struct PlanTransition {
	public:
		CompoundAction act;
		Formula condition;
	public:
		PlanTransition() {}
		PlanTransition(const CgTransition& trans) : act(trans.act), condition(trans.condition) {}
		PlanTransition(const CompoundAction& ca, const Formula& cond) : act(ca), condition(cond) {}

		bool operator==(const PlanTransition& other) const {
			return (act == other.act) && (condition == other.condition);
		}
	};

	inline std::ostream& operator<<(std::ostream& os, const PlanTransition& transition) {
		os << std::string("⟨");
		os << transition.act;
		os << ", ";
		os << transition.condition;
		os << "⟩";
		return os;
	}

}