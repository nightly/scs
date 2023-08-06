#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"
#include "scs/ConGolog/Program/while.h"

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

		virtual void AddTransition(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker,
		std::optional<std::shared_ptr<CgTransition>> transition_opt = std::nullopt) const override {
			p->AddTransition(graph, counter, tracker, transition_opt);

			// Remove any cond from second statement if a cond exists, e.g. if statement of a sequence, q doesn't need to enforce cond but p does
			auto transition = GetTransition(transition_opt);
			transition.get()->condition = true;
			q->AddTransition(graph, counter, tracker, transition);
		}

		virtual ProgramStep Step(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker,
		std::optional<std::shared_ptr<CgTransition>> transition_opt = std::nullopt) const override {
			// One step = do p, return q as remaining program if limiting to 1 step at a time for simultaneous concurrency
			return {};
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