#pragma once

#include <vector>

#include "scs/SituationCalculus/bat.h"
#include "scs/ConGolog/Program/configuration.h"
#include "scs/ConGolog/execution.h"

namespace scs {

	struct IProgram {
		virtual ~IProgram() = default;

		virtual void Decompose(Execution& exec) const = 0;
		virtual bool Final(const Situation& s) const = 0;

		virtual std::ostream& Print(std::ostream& os) const = 0;
	};

	inline std::ostream& operator<< (std::ostream& os, const IProgram& prog) {
		prog.Print(os);
		return os;
	}

}

// virtual IProgram* clone() const = 0;
