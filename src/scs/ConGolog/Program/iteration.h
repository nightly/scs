#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"

namespace scs {

	// Iteration (p*)
	struct Iteration : public IProgram {
		const IProgram* p;

		Iteration(const IProgram* p, const IProgram* q) 
			: p(p) {}

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