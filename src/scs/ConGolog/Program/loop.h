#pragma once

#include <queue>

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"
#include "scs/ConGolog/Program/while.h"
#include "scs/ConGolog/Program/nil.h"

namespace scs {

	// Shorthand for either \delta* or while(True) do (p) 
	struct Loop : public IProgram {
		std::shared_ptr<IProgram> p;

		Loop(const IProgram& p)
			: p(p.clone()) {}

		std::shared_ptr<IProgram> clone() const override {
			return std::make_shared<Loop>(*this);
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
