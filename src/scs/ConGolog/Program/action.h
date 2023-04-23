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
		
		bool Final(const Situation& s) const override {
			return false;
		}

		bool Trans(const Situation& s) const override {
			if (ca.IsSimple()) {
				return true; // TODO
			} else {
				return true; // TODO
			}
		}

		virtual std::vector<CompoundAction> Decompose(const Situation& s) const override {
			std::vector<CompoundAction> ret;
			
			// ...
			ret.emplace_back(ca);

			return ret;
		}

		std::ostream& Print(std::ostream& os) const override {
			os << "<Compound Action>" << ca;
			return os;
		}

	};

	inline std::ostream& operator<< (std::ostream& os, const ActionProgram& ap) {
		ap.Print(os);
		return os;
	}



}