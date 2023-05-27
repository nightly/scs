#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"

namespace scs {

	// Simultaneous/synchronized execution (p ||| q)
	struct Simultaneous : public IProgram {
		std::shared_ptr<IProgram> p;
		std::shared_ptr<IProgram> q;

		template<typename P, typename Q>
		Simultaneous(const P* p, const Q* q)
			: p(std::make_shared<P>(*p)), q(std::make_shared<Q>(*q)) {}

		template<typename P, typename Q>
		Simultaneous(const P& p, const Q& q)
			: p(std::make_shared<P>(p)), q(std::make_shared<Q>(q)) {}

		virtual void Decompose(Execution& exec) const override {
			// To support this with traces:
			// 1. Either allow CompoundActions in traces themselves
			// 2. Pass in two traces not 1 and then it can be done with Nop's added if one trace is shorter than the others

		}

		bool Final(const Situation& s) const override {
			return false;
		}


		std::ostream& Print(std::ostream& os) const override {
			os << "<Simultaneous>" << p << " ||| " << q;
			os << "\n";
			return os;
		}


	};

	inline std::ostream& operator<< (std::ostream& os, const Simultaneous& prog) {
		prog.Print(os);
		return os;
	}

}