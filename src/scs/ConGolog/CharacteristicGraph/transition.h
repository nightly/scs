#pragma once

#include <ostream>
#include <vector>

#include "scs/SituationCalculus/compound_action.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/Common/uuid.h"

namespace scs {

	struct CgTransition {
	public:
		CompoundAction act;
		std::vector<Variable> vars;
		Formula condition;
	private:
		UUID id_;
	public:
		CgTransition() {}
		CgTransition(const CompoundAction& ca, const Formula& cond) : act(ca), condition(cond) {}
		CgTransition(const CompoundAction& ca, const std::vector<Variable>& vars,
			const Formula& cond) : act(ca), vars(vars), condition(cond) {}

		bool HasVariables() const { return !vars.empty(); }
		size_t Id() const { return id_.Get(); }
		void SetId(UUID id) { id_ = id; }

		bool operator==(const CgTransition& other) const {
			return (act == other.act) && (vars == other.vars) && (condition == other.condition);
		}
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
		os << transition.act;
		os << ", ";
		os << transition.condition;
		os << "⟩";
		// os << transition.Id();
		return os;
	}

}