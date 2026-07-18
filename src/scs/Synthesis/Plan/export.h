#pragma once

#include <optional>
#include <string_view>
#include <filesystem>
#include <format>
#include <iostream>

#include "scs/Common/directory.h"
#include "scs/Synthesis/Plan/plan.h"

namespace scs {

	inline void ExportControllerToFile(const Plan& plan, const std::filesystem::path& path) {
		nightly::Styling style;
		nightly::ExportToFile(plan.lts, path, style, true);
	}

	inline void ExportController(const std::optional<Plan>& plan, std::string_view file_name = "controller") {
		if (!plan.has_value()) {
			return;
		}
		ExportControllerToFile(plan.value(), Path(file_name));
	}

}
