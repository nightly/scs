#pragma once

#include <string_view>
#include <filesystem>
#include <format>
#include <iostream>

#include "scs/Common/directory.h"
#include "scs/Synthesis/topology/interface_topology.h"

namespace scs {

	inline void ExportTopology(const ITopology& topology, std::string_view file_name = "topology", std::string_view dir = "../../exports/") {
		nightly::Styling style;
		std::filesystem::path path{Path(file_name, dir)};
		nightly::ExportToFile(topology.lts(), path, style, true);
	}

}