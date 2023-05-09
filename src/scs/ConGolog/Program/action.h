#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"
#include "scs/ConGolog/Program/check.h"

namespace scs {


	struct ActionProgram : public IProgram {
	public:
		CompoundAction ca;
	public:
		ActionProgram(const CompoundAction& act)
			: ca(act) {}

		ActionProgram(const Action& act) 
			: ca(act) {}
		
		virtual std::vector<CompoundAction> Decompose(const Situation& s) const override {
			std::vector<CompoundAction> ret;

			if (ca.IsSimple()) {
				ret.emplace_back(ca.Actions()[0]);
			} else {

			}

			return ret;
		}

		bool Final(const Situation& s) const override {
			return false;
		}

		std::ostream& Print(std::ostream& os) const override {
			// os << "<Compound Action>" << ca;
			return os;
		}

	};

	inline std::ostream& operator<< (std::ostream& os, const ActionProgram& ap) {
		ap.Print(os);
		return os;
	}



}