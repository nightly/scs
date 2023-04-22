#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"

namespace scs {

	// Interleaved concurrency (p || q)
	struct Interleaved : public IProgram {
		const IProgram* p;
		const IProgram* q;

		Interleaved(const IProgram* p, const IProgram* q) : 
			p(p), q(q) {}

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