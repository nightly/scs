#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"

namespace scs {

	struct Check : public IProgram {
		Formula holds;

		Check(const Formula& f) : holds(f) {}
		Check(Formula&& f) : holds(std::move(f)) {}

		virtual void Decompose(Execution& exec) const override {
			exec.trace.emplace_back(holds);
		}

		bool Final(const Situation& s) const override {
			return false;
		}

		std::ostream& Print(std::ostream& os) const override {
			os << "<Check> " << holds;
			os << "\n";
			return os;
		}

	};

	inline std::ostream& operator<< (std::ostream& os, const Check& check_prog) {
		check_prog.Print(os);
		return os;
	}

}