#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"
#include "scs/ConGolog/Program/nil.h"

namespace scs {

	// Iteration (p*)
	struct Iteration : public IProgram {
		std::shared_ptr<IProgram> p;

		Iteration(const IProgram* p)
			: p(p->clone()) {}

		Iteration(const IProgram& p)
			: p(p.clone()) {}

		std::shared_ptr<IProgram> clone() const override {
			return std::make_shared<Iteration>(*this);
		}

		virtual void AddTransition(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker,
		std::optional<std::shared_ptr<CgTransition>> transition_opt = std::nullopt) const override {

		}

		virtual ProgramStep Step(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker,
		std::optional<std::shared_ptr<CgTransition>> transition_opt = std::nullopt) const override {
			return {};
		}

		std::ostream& Print(std::ostream& os) const override {
			os << "<NonDetIteration>" << *p << "*";
			os << "\n";
			return os;
		}

	};

	inline std::ostream& operator<< (std::ostream& os, const Iteration& prog) {
		prog.Print(os);
		return os;
	}

}