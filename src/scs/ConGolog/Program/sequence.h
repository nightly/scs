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
			q->AddTransition(graph, counter, tracker, transition_opt);
		}

		virtual ProgramStep Step(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker,
		std::optional<std::shared_ptr<CgTransition>> transition_opt = std::nullopt) const override {
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