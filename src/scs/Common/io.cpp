#include "scs/Common/io.h"

#include <filesystem>
#include <ostream>
#include <string>
#include <string_view>
#include <fstream>
#include <streambuf>

#include "scs/Common/log.h"

namespace scs {

	bool AreFilesEqual(const std::filesystem::path& filepath_1, const std::filesystem::path& filepath_2) {
		std::ifstream file_1(filepath_1);
		std::ifstream file_2(filepath_2);
	
		if (!file_1.is_open() || !file_2.is_open()) {
			SCS_CRITICAL("Unable to open one of the files: {} = {},  {} = {}", filepath_1.string(), file_1.is_open(), 
				filepath_2.string(), file_2.is_open());
			return false;
		}

		std::string file_1_contents{std::istreambuf_iterator<char>(file_1), std::istreambuf_iterator<char>()};
		std::string file_2_contents{std::istreambuf_iterator<char>(file_2), std::istreambuf_iterator<char>()};

		return file_1_contents == file_2_contents;
	}

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