#pragma once

#include <filesystem>
#include <ostream>
#include <fstream>
#include <string>
#include <string_view>
#include <filesystem>
#include <vector>

namespace scs {

	bool AreFilesEqual(const std::filesystem::path& filepath_1, const std::filesystem::path& filepath_2);

	 std::string ReadIntoString(const std::filesystem::path& path);
	 void ReadAll(const std::filesystem::path& path, std::string_view extension);
	

}