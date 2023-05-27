#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"
#include "scs/ConGolog/Program/while.h"

namespace scs {

	// Shorthand for while(True) do (p)
	struct Loop : public IProgram {
		Formula check;
		std::shared_ptr<IProgram> p;

		template<typename P>
		Loop(const P* p)
			: check(true), p(std::make_shared<P>(*p)) {}

		template<typename P>
		Loop(const Formula& f, const P& p)
			: check(true), p(std::make_shared<P>(p)) {}

		virtual void Decompose(Execution& exec) const override {
			// Basically just use while(True) do(p) construct

		}

		bool Final(const Situation& s) const override {
			return false;
		}
	};

	inline std::ostream& operator<< (std::ostream& os, const Loop& prog) {
		prog.Print(os);
		return os;
	}


}