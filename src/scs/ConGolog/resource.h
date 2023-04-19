#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/SituationCalculus/bat.h"

namespace scs {

	struct Resource {
	public:
		IProgram* program;
		BasicActionTheory bat;
	public:

	};

}