#pragma once

#include "scs/Synthesis/Exact/model.h"

namespace scs::examples {

	struct AssemblyCosts {
		uint64_t base = 1;
		uint64_t manufacturing = 1;
		uint64_t nop = 1;
		uint64_t transfer = 2;
		uint64_t preparatory = 2;
	};

	SynthesisProblem MakeAssemblyProblem(AssemblyCosts costs = {});
	SynthesisOptions AssemblySynthesisOptions(
		WorklistOrder worklist = WorklistOrder::BreadthFirst);

}
