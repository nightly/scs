#pragma once

#include <filesystem>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace scs {

	 std::string ReadIntoString(const std::filesystem::path& path);

	 void ReadAll(const std::filesystem::path& path, std::string_view extension);
	

}