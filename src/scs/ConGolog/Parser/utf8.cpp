#include <string_view>

namespace scs {

	[[nodiscard]] size_t NumberOfBytes(std::string_view view) {
		if (view.empty()) {
			return 0;
		}
		size_t bytes = 1;
		while (view.size() > bytes && (view[bytes] & 0b11000000) == 0b10000000) {
			++bytes;
		}
		return bytes;
	}

}