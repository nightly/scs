#include "scs/Common/directory.h"

#include <filesystem>
#include <ostream>

namespace scs {

	/*
	 * @brief Creates the parent directory/directories for a given path if it doesn't already exist
	 */
	void CreateDirectoryForPath(const std::filesystem::path& path) {
		if (!std::filesystem::is_directory(path.parent_path()) && path.has_parent_path()) {
			std::filesystem::create_directories(path.parent_path());
		}
	}

	std::filesystem::path GetCurrentDir() {
		return std::filesystem::current_path();
	}

	/*
	 * @param os: Default = std::cout
	 */
	void PrintCurrentDir(std::ostream& os) {
		os << GetCurrentDir() << std::endl;
	}


}