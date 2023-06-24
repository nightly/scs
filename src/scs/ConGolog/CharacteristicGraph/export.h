#pragma once

#include <string_view>
#include <filesystem>
#include <format>
#include <iostream>

#include "scs/Common/directory.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"

namespace scs {

	static inline std::filesystem::path Path(std::string_view file_name) {
		std::string combined_str = std::format("../../exports/{}.gv", file_name);
		return combined_str;
	}

	static void DotOutput(const std::filesystem::path& path, const std::string& extension = "svg") {
		std::filesystem::current_path(path.parent_path());
		std::string cmd = std::format("dot -T{} {} -o {}.{}", extension, path.filename().string(),
			path.stem().string(), extension);
		system(cmd.c_str());
	}

	inline void ExportResourceGraph(const CharacteristicGraph& cg, std::string_view file_name) {
		nightly::Styling style;
		std::filesystem::path path{Path(file_name)};
		nightly::ExportToFile(cg.lts, path, style, true);
		DotOutput(path);
	}

	inline void ExportRecipeGrpah(const CharacteristicGraph& cg, std::string_view file_name) {
		nightly::Styling style;
		std::filesystem::path path{Path(file_name)};
		nightly::ExportToFile(cg.lts, path, style, true);
		DotOutput(path);
	}

}