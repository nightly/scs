#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"

namespace scs {

	// Sequence (p;q)
	struct Sequence : public IProgram {
		std::shared_ptr<IProgram> p;
		std::shared_ptr<IProgram> q;

		template<typename P, typename Q>
		Sequence(const P* p, const Q* q)
			: p(std::make_shared<P>(*p)), q(std::make_shared<Q>(*q)) {}

		template<typename P, typename Q>
		Sequence(const P& p, const Q& q)
			: p(std::make_shared<P>(p)), q(std::make_shared<Q>(q)) {}

		virtual void Decompose(Execution& exec) const override {
			p->Decompose(exec);
			q->Decompose(exec);
		}

		bool Final(const Situation& s) const override {
			return false;
		}

		std::ostream& Print(std::ostream& os) const override {
			os << "<Sequence> " << *p << " ; " << *q;
			os << "\n";
			return os;
		}
	};

	inline std::ostream& operator<< (std::ostream& os, const Sequence& prog) {
		prog.Print(os);
		return os;
	}

}