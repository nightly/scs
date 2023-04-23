#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/fol.h"


namespace scs {

	struct Check : public IProgram {
		Formula holds;

		Check(const Formula& f) 
			: holds(f) {}

		bool Final(const Situation& s) const override {
			return false;
		}

		bool Trans(const Situation& s) const override {
			return false;
		}

		virtual std::vector<CompoundAction> Decompose(const Situation& s) const override {
			std::vector<CompoundAction> ret;

			return ret;
		}

		std::ostream& Print(std::ostream& os) const override {
			os << "<Holds>" << holds;
			os << "\n";
			return os;
		}


	};

	inline std::ostream& operator<< (std::ostream& os, const Check& prog) {
		prog.Print(os);
		return os;
	}


}