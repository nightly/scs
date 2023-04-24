#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"
#include "scs/ConGolog/Program/while.h"
#include "scs/ConGolog/Program/check.h"

namespace scs {

	// Shorthand for while(True) do (p)
	struct Loop : public IProgram {
		Check check;
		std::shared_ptr<IProgram> p;

		template<typename P>
		Loop(const Formula& f, const P* p)
			: check(f), p(std::make_shared<P>(*p)) {}

		template<typename P>
		Loop(const Formula& f, const P& p)
			: Check(f), p(std::make_shared<P>(p)) {}

		virtual std::vector<CompoundAction> Decompose(const Situation& s) const override {
			std::vector<CompoundAction> ret;

			return ret;
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