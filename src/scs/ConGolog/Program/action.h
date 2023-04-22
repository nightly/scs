#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"
#include "scs/ConGolog/Program/check.h"

namespace scs {

	// @Correctness: Should this be a CompoundAction?

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
				return false; // TODO
			} else {
				return false; // TODO
			}
		}

		virtual std::vector<Configuration> Transmute(const Situation& s) const override {
			std::vector<Configuration> ret;

			return ret;
		}
	};


}