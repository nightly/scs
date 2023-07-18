#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"

#include "scs/Memory/box.h"
#include "scs/ConGolog/Program/nil.h"

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

		virtual void AddTransition(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker,
		std::optional<std::shared_ptr<CgTransition>> transition_opt = std::nullopt) const override {
			auto transition = GetTransition(transition_opt);

			StateTracker t1(tracker), t2(tracker);
			p->AddTransition(graph, counter, t1, transition);
			q->AddTransition(graph, counter, t2, transition);
			tracker = t1 + t2;
		}

		virtual std::shared_ptr<IProgram> Step(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker,
		std::optional<std::shared_ptr<CgTransition>> transition_opt = std::nullopt) const override {
			return std::make_shared<Nil>();
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