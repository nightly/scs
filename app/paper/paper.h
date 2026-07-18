#pragma once

#include <string_view>

namespace scs::paper {

	enum class Suite {
		All,
		Tables,
		Grounding,
		Controllers,
		Limits,
		Scaling,
		AStarWorker,
	};

	int Run(int argc, char** argv);
	std::string_view SuiteName(Suite suite);

}
