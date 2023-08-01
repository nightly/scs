#pragma once

#include <vector>
#include <map>
#include <ankerl/unordered_dense.h>

namespace scs {

	template <typename T>
	size_t Duplicates(const std::vector<T>& vec) {
		ankerl::unordered_dense::map<T, size_t> count_map;
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