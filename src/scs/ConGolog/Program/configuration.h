#pragma once

#include "scs/SituationCalculus/situation.h"

namespace scs {

	class IProgram;

	struct Configuration {
		const IProgram* prog;
		Situation sit;

		Configuration(const IProgram* prog, const Situation& s);
		Configuration(const IProgram* prog, Situation&& s);

	};

}