#pragma once

#include <string>
#include <vector>
#include <set>

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"

namespace scs {

	struct Pick : public IProgram {
		std::unordered_set<Variable> args;
		std::shared_ptr<IProgram> p;

		template<typename P>
		Pick(std::unordered_set<Variable>& args, const P* p)
			: args(args), p(std::make_shared<P>(*p)) {}

		template<typename P>
		Pick(std::unordered_set<Variable>& args, const P& p)
			: args(args), p(std::make_shared<P>(p)) {}

		virtual std::vector<CompoundAction> Decompose(const Situation& s) const override {
			std::vector<CompoundAction> ret;

			return ret;
		}

		bool Final(const Situation& s) const override {
			return false;
		}

		std::ostream& Print(std::ostream& os) const override {
			os << "<Pick>";
			os << "	<From> \n";
			os << " <To> " << p << "\n";
			return os;
		}

	};

	inline std::ostream& operator<< (std::ostream& os, const Pick& prog) {
		prog.Print(os);
		return os;
	}


}