#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"

namespace scs {

	// Interleaved concurrency (p || q)
	struct Interleaved : public IProgram {
		std::shared_ptr<IProgram> p;
		std::shared_ptr<IProgram> q;

		Interleaved(const IProgram* p, const IProgram* q)
			: p(p->clone()), q(q->clone()) {}

		Interleaved(const IProgram& p, const IProgram& q)
			: p(p.clone()), q(q.clone()) {}

		std::shared_ptr<IProgram> clone() const override {
			return std::make_shared<Interleaved>(*this);
		}

		virtual void AddTransition(CharacteristicGraph& graph, StateCounter& counter,
		StateTracker& tracker, CgTransition transition = CgTransition(), int loop_back = -1) const override {

		}

		std::ostream& Print(std::ostream& os) const override {
			os << "<Interleaved>" << p << " || " << q;
			os << "\n";
			return os;
		}

	};

	inline std::ostream& operator<< (std::ostream& os, const Interleaved& prog) {
		prog.Print(os);
		return os;
	}

}