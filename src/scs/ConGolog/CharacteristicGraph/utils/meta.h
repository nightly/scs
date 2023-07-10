#pragma once

#include <vector>
#include <optional>

namespace scs {

	struct StateMeta {
	public:
		std::optional<std::vector<size_t>> loop_back = std::nullopt;
	};

}