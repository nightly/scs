#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/ConGolog/Program/check.h"
#include "scs/FirstOrderLogic/formula.h"

namespace scs {

	struct CgWhile : public IProgram {
	public:
		Check condition;
		const IProgram* do_prog;
	public:
		CgWhile(const Check& condition, const IProgram* do_prog)
			: condition(condition), do_prog(do_prog) {}

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