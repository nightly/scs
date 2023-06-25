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

		virtual void AddTransition(CharacteristicGraph& graph, StateCounter& counter,
		StateTracker& tracker, CgTransition transition = CgTransition(), int loop_back = -1) const override {
			size_t first = counter.Increment();
			CgTransition first_transition{first};
			p->AddTransition(graph, counter, tracker, first_transition);

			tracker.SetState(first); // Move forward one state
			size_t second = counter.Increment();
			CgTransition second_transition{second};
			q->AddTransition(graph, counter, tracker, second_transition);
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