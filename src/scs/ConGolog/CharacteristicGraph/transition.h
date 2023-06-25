#pragma once

#include <ostream>
#include <vector>

#include "scs/SituationCalculus/compound_action.h"
#include "scs/FirstOrderLogic/fol.h"

namespace scs {

	struct CgTransition {
	public:
		CompoundAction act;
		std::vector<Object> vars;
		Formula condition;
	public:
		CgTransition() {}
		CgTransition(const CompoundAction& ca, const Formula& cond) : act(ca), condition(cond) {}
		CgTransition(const CompoundAction& ca, const std::vector<Object>& objs, 
			const Formula& cond) : act(ca), vars(objs), condition(cond) {}

		bool HasVariables() const { return !vars.empty(); }
	};

	inline std::ostream& operator<<(std::ostream& os, const CgTransition& transition) {
		os << std::string("⟨");
		if (transition.HasVariables()) {
			os << "π ";
			for (size_t i = 0; i < transition.vars.size(); ++i) {
				os << transition.vars[i];
				if (i != transition.vars.size() - 1) {
					os << ",";
				}
			}
		}
		os << "{";
		os << transition.act;
		os << "},";
		os << transition.condition;
		os << "⟩";
		return os;
	}

}