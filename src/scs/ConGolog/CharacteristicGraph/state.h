#pragma once

#include <string>

namespace scs {

	inline std::ostream& operator<<(std::ostream& os, size_t s) {
		os << std::to_string(s);
		return os;
	}	

}