#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"

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

		virtual void AddTransition(CharacteristicGraph& graph, StateCounter& counter,
		StateTracker& tracker, CgTransition transition = CgTransition(), int loop_back = -1) const override {
		
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