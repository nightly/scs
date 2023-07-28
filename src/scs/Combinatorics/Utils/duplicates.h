#pragma once

#include <vector>
#include <map>
#include <unordered_map>

namespace scs {

	template <typename T>
	size_t Duplicates(const std::vector<T>& vec) {
		std::unordered_map<T, size_t> count_map;
		for (const auto& el : vec) {
			count_map[el]++;
		}

		size_t duplicates = 0;
		for (const auto& [key, val] : count_map) {
			if (val > 1) {
				duplicates++;
			}
		}
		return duplicates;
	}

}