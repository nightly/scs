#pragma once

#include <filesystem>
#include <ostream>
#include <iostream>

namespace scs {

	void CreateDirectoryForPath(const std::filesystem::path& path);
	std::filesystem::path GetCurrentDir();
	void PrintCurrentDir(std::ostream& os = std::cout);

}