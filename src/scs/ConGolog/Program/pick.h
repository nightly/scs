#pragma once

#include <string>
#include <vector>
#include <set>

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"

namespace scs {

	struct Pick : public IProgram {
		std::unordered_set<Variable> args;
		const IProgram* p;

		Pick(std::unordered_set<Variable>& args, const IProgram* p) 
			: args(args), p(p) {}

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