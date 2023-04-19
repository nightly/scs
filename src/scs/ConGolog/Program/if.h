#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"
#include "scs/ConGolog/Program/check.h"

namespace scs {

	struct CgIf : public IProgram {
	public:
		Check check;
		const IProgram* p; // where condition is true
		const IProgram* q; // where condition is false (else block)

	public:
		CgIf(const Check& check, const IProgram* p, const IProgram* q) 
			: check(check), p(p), q(q) {} // Could also be std::move'd the three

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