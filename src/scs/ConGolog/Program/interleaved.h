#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"
#include "scs/ConGolog/Program/nil.h"

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

		virtual void AddTransition(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker,
		std::optional<std::shared_ptr<CgTransition>> transition_opt = std::nullopt) const override {
			StateTracker tracker_1(tracker), tracker_2(tracker);

			// @Incomplete: switch to Step() instead of AddTransition()
			if (p != std::make_shared<Nil>()) {
				p->AddTransition(graph, counter, tracker_1, transition_opt);
				q->AddTransition(graph, counter, tracker_1, transition_opt);
			}
			if (q != std::make_shared<Nil>()) {
				p->AddTransition(graph, counter, tracker_2, transition_opt);
				q->AddTransition(graph, counter, tracker_2, transition_opt);
			}
			tracker = tracker_1 + tracker_2;
		}

		std::ostream& Print(std::ostream& os) const override {
			os << "<Interleaved>" << p << " || " << q;
			os << "\n";
			return os;
		}

		virtual std::shared_ptr<IProgram> Step(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker,
		std::optional<std::shared_ptr<CgTransition>> transition_opt = std::nullopt) const override {
			return std::make_shared<Nil>();
		}

	};

	inline std::ostream& operator<< (std::ostream& os, const Interleaved& prog) {
		prog.Print(os);
		return os;
	}

}