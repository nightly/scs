#pragma once

#include <optional>
#include <string_view>
#include <filesystem>
#include <format>
#include <iostream>

#include "scs/Common/directory.h"
#include "scs/Synthesis/plan.h"

namespace scs {

	inline void ExportController(const std::optional<Plan>& plan, std::string_view file_name = "controller") {
		if (!plan.has_value()) {
			return;
		}
		nightly::Styling style;
		std::filesystem::path path{Path(file_name)};
		nightly::ExportToFile(plan.value().lts, path, style, true);
	}

}