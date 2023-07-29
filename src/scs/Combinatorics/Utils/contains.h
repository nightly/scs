#pragma once

#include <vector>

namespace scs {

	template <typename T>
	bool Contains(const std::vector<T>& vec, const T& find) {
		for (const auto& element : vec) {
			if (element == find) {
				return true;
			}
		}
		return false;
	}
	
}