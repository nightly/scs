#pragma once

#include <vector>

#include "scs/SituationCalculus/bat.h"
#include "scs/ConGolog/execution.h"

namespace scs {

	struct IProgram {
		virtual ~IProgram() = default;

		virtual std::shared_ptr<IProgram> clone() const = 0;

		virtual void Decompose(Execution& exec) const = 0;

		virtual std::ostream& Print(std::ostream& os) const = 0;
	};

	inline std::ostream& operator<< (std::ostream& os, const IProgram& prog) {
		prog.Print(os);
		return os;
	}

}

// virtual IProgram* clone() const = 0;
