#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"

namespace scs {

	// Iteration (p*)
	struct Iteration : public IProgram {
		std::shared_ptr<IProgram> p;

		template<typename P>
		Iteration(const P* p)
			: p(std::make_shared<P>(*p)) {}

		template<typename P>
		Iteration(const P& p)
			: p(std::make_shared<P>(p)) {}

		virtual void Decompose(Execution& exec) const override {
			Execution e1;
			e1.non_det_iterative_ = true;
			p->Decompose(e1);
			exec.sub_executions.emplace_back(e1);
		}

		bool Final(const Situation& s) const override {
			return false;
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