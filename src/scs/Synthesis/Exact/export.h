#pragma once

#include <filesystem>
#include <string>

#include "scs/Synthesis/Exact/solver.h"

namespace scs {

	std::string ControllerToGraphViz(const Controller& controller);
	std::string ControllerToTikz(const Controller& controller);
	void ExportControllerGraphViz(const Controller& controller, const std::filesystem::path& path);
	void ExportControllerTikz(const Controller& controller, const std::filesystem::path& path);

}
