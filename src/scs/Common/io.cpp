#include "scs/Common/io.h"

#include <filesystem>
#include <ostream>
#include <string>
#include <string_view>

namespace scs {

	/*
	 * @brief: Removes UTF-8 byte order mark (BOM) if present 
	 */
	static void RemoveBOM(std::string& str) {
		if (str.compare(0, 3, "\xEF\xBB\xBF") == 0) {
			str.erase(0, 3); // Remove
		}
	}

	
	[[nodiscard]] std::string ReadIntoString(const std::filesystem::path& path) {
		std::ifstream stream(path, std::ios::in | std::ios::binary);
		const auto size = std::filesystem::file_size(path);
		std::string ret(size, '\0');
		stream.read(ret.data(), size);
		RemoveBOM(ret);
		return ret;
	}

	void ReadAll(const std::filesystem::path& path, std::string_view extension) {
		for (const auto& file : std::filesystem::directory_iterator(path)) {
			if (file.path().filename().extension() == extension) {
               
			}		
		}
	}

}