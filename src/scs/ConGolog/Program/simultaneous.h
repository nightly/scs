#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"
#include "scs/ConGolog/Program/while.h"

namespace scs {

	// Simultaneous/synchronized execution (p ||| q)
	struct Simultaneous : public IProgram {
		std::shared_ptr<IProgram> p;
		std::shared_ptr<IProgram> q;

		Simultaneous(const IProgram* p, const IProgram* q)
			: p(p->clone()), q(q->clone()) {}

		Simultaneous(const IProgram& p, const IProgram& q)
			: p(p.clone()), q(q.clone()) {}

		Simultaneous(const std::shared_ptr<IProgram> p, const std::shared_ptr<IProgram> q)
			: p(p), q(q) {}

		std::shared_ptr<IProgram> clone() const override {
			return std::make_shared<Simultaneous>(*this);
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
