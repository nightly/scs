#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"

namespace scs {

	// Iteration (p*)
	struct Iteration : public IProgram {
		std::shared_ptr<IProgram> p;

		template<typename P>
		Iteration(const P* p)
			: p(std::make_shared<P>(*p)) {}

		template<typename P>
		Iteration(const P& p)
			: p(std::make_shared<P>(p)) {}

		virtual std::vector<CompoundAction> Decompose(const Situation& s) const override {
			std::vector<CompoundAction> ret;

			auto decomposition_p = p->Decompose(s);
			// @Incomplete
			ret.insert(ret.end(), decomposition_p.begin(), decomposition_p.end());

			return ret;
		}

		bool Final(const Situation& s) const override {
			return false;
		}

		std::ostream& Print(std::ostream& os) const override {
			os << "<NonDetIteration>" << *p << "*";
			os << "\n";
			return os;
		}

	};

	inline std::ostream& operator<< (std::ostream& os, const Iteration& prog) {
		prog.Print(os);
		return os;
	}

}