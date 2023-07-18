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

		std::shared_ptr<IProgram> clone() const override {
			return std::make_shared<Simultaneous>(*this);
		}

		virtual void AddTransition(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker,
		std::optional<std::shared_ptr<CgTransition>> transition_opt = std::nullopt) const override {
			//StateTracker tracker_1(tracker), tracker_2(tracker);
			//CgTransition transition_step(transition);

			//auto ret_p = p->Step(graph, counter, tracker_1, transition_step);
			//auto ret_q = q->Step(graph, counter, tracker_2, transition_step);

			//if (ret_p != std::make_shared<Nil>()) {
			//	p->Step(graph, counter, tracker_1, transition);
			//}
			//if (ret_q != std::make_shared<Nil>()) {

			//}
			//tracker = tracker_1 + tracker_2;
		}

		virtual std::shared_ptr<IProgram> Step(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker,
		std::optional<std::shared_ptr<CgTransition>> transition_opt = std::nullopt) const override {
			return std::make_shared<Nil>();
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