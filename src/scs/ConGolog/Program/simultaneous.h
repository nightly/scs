#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"

namespace scs {

	// Simultaneous/synchronized execution (p ||| q)
	struct Simultaneous : public IProgram {
		std::shared_ptr<IProgram> p;
		std::shared_ptr<IProgram> q;

		Simultaneous(const IProgram* p, const IProgram* q)
			: p(p->clone()), q(q->clone()) {}

		Simultaneous(const IProgram& p, const IProgram& q)
			: p(p.clone()), q(q.clone()) {}

		std::shared_ptr<IProgram> clone() const override {
			return std::make_shared<Simultaneous>(*this);
		}

		//virtual void Decompose(Execution& exec) const override {
		//	// To support this with traces:
		//	// 1. Either allow CompoundActions in traces themselves
		//	// 2. Pass in two traces not 1 and then it can be done with Nop's added if one trace is shorter than the others

		//}

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