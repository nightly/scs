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

		virtual void AddTransition(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker,
		std::optional<std::shared_ptr<CgTransition>> transition_opt = std::nullopt) const override {
			auto transition = GetTransition(transition_opt);
			
			ProgramStep p_prime = p->Step(graph, counter, tracker, transition);
			ProgramStep q_prime = q->Step(graph, counter, tracker, p_prime.evolved_transition);
			size_t next = counter.Increment();
			for (const auto& current : tracker.CurrentStates()) {
				q_prime.evolved_transition->SetId(UUID());
				graph.lts.AddTransition(current, *q_prime.evolved_transition, next);
			}
			tracker.SetState(next);

			if (*p_prime.evolved_program != Nil() || *q_prime.evolved_program != Nil()) {
				auto next_prog = Simultaneous(p_prime.evolved_program, q_prime.evolved_program);
				next_prog.AddTransition(graph, counter, tracker);
			}
		}

		virtual ProgramStep Step(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker,
		std::optional<std::shared_ptr<CgTransition>> transition_opt = std::nullopt) const override {
			return {};
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