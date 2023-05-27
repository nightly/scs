#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"

namespace scs {


	struct ActionProgram : public IProgram {
	public:
		CompoundAction ca;
	public:
		ActionProgram(const CompoundAction& act)
			: ca(act) {}

		ActionProgram(const Action& act) 
			: ca(act) {}
		
		virtual void Decompose(Execution& exec) const override {
			if (ca.IsSimple()) {
				exec.trace.emplace_back(ca.Actions()[0]);
			} else {

			}
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