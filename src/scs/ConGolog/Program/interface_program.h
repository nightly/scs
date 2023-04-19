#pragma once

#include <vector>

#include "scs/SituationCalculus/bat.h"
#include "scs/ConGolog/Program/configuration.h"

namespace scs {

	struct IProgram {
		virtual ~IProgram() = default;

		virtual bool Trans(const Situation& s) const = 0;
		virtual bool Final(const Situation& s) const = 0;

		virtual std::vector<Configuration> Transmute(const Situation& s) const = 0;
	};

}