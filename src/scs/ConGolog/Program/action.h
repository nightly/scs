#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"
#include "scs/ConGolog/Program/check.h"

namespace scs {

	// @Correctness: Should this be a CompoundAction?

	struct ActionProgram : public IProgram {
	public:
		Action action;
	public:
		ActionProgram(const Action& act) 
			: action(act) {}
		
		bool Final(const Situation& s) const override {
			return false;
		}

		bool Trans(const Situation& s) const override {
			return Check{true}.Trans(s);
		}

		virtual std::vector<Configuration> Transmute(const Situation& s) const override {
			std::vector<Configuration> ret;

			return ret;
		}
	};


}