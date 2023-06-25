#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"

#include "scs/Memory/box.h"

namespace scs {

	// Non-deterministic branching (p | q)
	struct Branch : public IProgram {
		std::shared_ptr<IProgram> p;
		std::shared_ptr<IProgram> q;

		Branch(const IProgram* p, const IProgram* q)
			: p(p->clone()), q(q->clone()) {}

		Branch(const IProgram& p, const IProgram& q)
			: p(p.clone()), q(q.clone()) {}

		std::shared_ptr<IProgram> clone() const override {
			return std::make_shared<Branch>(*this);
		}

		virtual void AddTransition(CharacteristicGraph& graph, StateCounter& counter,
		StateTracker& tracker, CgTransition transition = CgTransition(), int loop_back = -1) const override {
			transition.condition = true;
			p->AddTransition(graph, counter, tracker, transition, loop_back);
			q->AddTransition(graph, counter, tracker, transition, loop_back);
		}

		std::ostream& Print(std::ostream& os) const override {
			os << "<NonDet>" << *p << " | " << *q;
			os << "\n";
			return os;
		}

	};

	inline std::ostream& operator<< (std::ostream& os, const Branch& br) {
		br.Print(os);
		return os;
	}

}