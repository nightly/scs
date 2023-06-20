#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"

namespace scs {

	// Sequence (p;q)
	struct Sequence : public IProgram {
		std::shared_ptr<IProgram> p;
		std::shared_ptr<IProgram> q;

		Sequence(const IProgram* p, const IProgram* q)
			: p(p->clone()), q(q->clone()) {}

		Sequence(const IProgram& p, const IProgram& q)
			: p(p.clone()), q(q.clone()) {}

		std::shared_ptr<IProgram> clone() const override {
			return std::make_shared<Sequence>(*this);
		}

		virtual void Decompose(Execution& exec) const override {
			p->Decompose(exec);
			q->Decompose(exec);
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