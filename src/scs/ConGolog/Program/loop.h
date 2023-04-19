#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"
#include "scs/ConGolog/Program/while.h"
#include "scs/ConGolog/Program/check.h"

namespace scs {

	// Shorthand for while(True) do (p)
	struct Loop : public IProgram {
		Check check;
		const IProgram* p;

		Loop(const IProgram* p, const Formula& check) 
			: p(p), check(check) {} // can also std::move the check

		bool Final(const Situation& s) const override {
			return false;
		}

		bool Trans(const Situation& s) const override {
			return false;
		}

		virtual std::vector<Configuration> Transmute(const Situation& s) const override {
			std::vector<Configuration> ret;

			return ret;
		}
	};


}