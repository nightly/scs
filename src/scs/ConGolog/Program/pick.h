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

		virtual void Decompose(Execution& exec) const override {
			// Basically needs a special signification of pick variable that should be substituted at planning time

		}

		bool Final(const Situation& s) const override {
			return false;
		}

		std::ostream& Print(std::ostream& os) const override {
			os << "<Pick>";
			os << "	<From> ";
			for (auto it = args.begin(); it != args.end(); ++it) {
				if (it != args.begin()) {
					os << ",";
				}
				os << *it;
			}
			os << "\n";
			os << " <To> " << *p << "\n";
			return os;
		}

	};

	inline std::ostream& operator<< (std::ostream& os, const Pick& prog) {
		prog.Print(os);
		return os;
	}


}