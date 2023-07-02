#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"
#include "scs/ConGolog/Program/while.h"

namespace scs {

	// Shorthand for while(True) do (p)
	struct Loop : public IProgram {
		Formula check;
		std::shared_ptr<IProgram> p;

		Loop(const IProgram& p)
			: check(true), p(p.clone()) {}

		std::shared_ptr<IProgram> clone() const override {
			return std::make_shared<Loop>(*this);
		}

		virtual void AddTransition(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker,
		StateMeta& meta, CgTransition transition = CgTransition()) const override {
		
		}

		std::ostream& Print(std::ostream& os) const override {
			os << "<Loop>" << *p << "*";
			os << "\n";
			return os;
		}
	};

	inline std::ostream& operator<< (std::ostream& os, const Loop& prog) {
		prog.Print(os);
		return os;
	}


}