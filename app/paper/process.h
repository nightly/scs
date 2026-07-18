#pragma once

#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

namespace scs::paper {

	struct ChildResult {
		int exit_code = -1;
		bool timed_out = false;
		bool killed = false;
		double wall_seconds = 0.0;
	};

	ChildResult RunChild(const std::filesystem::path& executable,
		const std::vector<std::string>& arguments, std::chrono::milliseconds timeout,
		std::chrono::milliseconds grace = std::chrono::seconds(5));

}
