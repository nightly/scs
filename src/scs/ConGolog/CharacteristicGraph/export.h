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

	static void GenerateImagesFromDot(const std::filesystem::path& dir_path = "../../exports/", const std::string& extension = "svg") {
		std::filesystem::current_path(dir_path);
		for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path())) {
			if (entry.path().extension() == ".gv") {
				std::string cmd = std::format("dot -T{} {} -o {}.{}", extension, entry.path().filename().string(),
					entry.path().stem().string(), extension);
				system(cmd.c_str());
			}
		}

	}

	inline void ExportGraph(const CharacteristicGraph& cg, std::string_view file_name) {
		nightly::Styling style;
		std::filesystem::path path{Path(file_name)};
		nightly::ExportToFile(cg.lts, path, style, true);
	}
	

}