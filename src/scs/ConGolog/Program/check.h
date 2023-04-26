#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/fol.h"


namespace scs {

	struct Check : public IProgram {
		Formula holds;

		Check(const Formula& f) 
			: holds(f) {}

		virtual std::vector<ActionState> Decompose(const Situation& s) const override {
			// Trans(Check) is False, synchronized
			std::vector<ActionState> ret;
			return ret;
		}

		bool Final(const Situation& s) const override {
			scs::Evaluator eval{ s };
			return std::visit(eval, holds);
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