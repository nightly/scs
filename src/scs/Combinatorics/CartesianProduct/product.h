#pragma once

#include <vector>
#include <ostream>
#include "scs/Common/print.h"

namespace scs {

	template <typename T>
    std::vector<std::vector<T>> Product(std::vector<const std::vector<T>*> input_vectors, const T& flag_value = T()) {
        // Handle any vectors that are empty by replacing with a vector of 1 flag value
        std::vector<T> flag_vec = { flag_value };
        for (auto& vec : input_vectors) {
            if (vec->empty()) {
                vec = &flag_vec;
            }
        }

        // Calculate cartesian product
        std::vector<std::vector<T>> accumulated_product = { {} };
        for (const auto& current_vec : input_vectors) {
            std::vector<std::vector<T>> temp_product;
            temp_product.reserve(accumulated_product.size() * current_vec->size());
            for (const auto& existing_combo : accumulated_product) {
                for (const auto& current_value : *current_vec) {
                    temp_product.emplace_back(existing_combo);
                    temp_product.back().emplace_back(current_value);
                }
            }
            accumulated_product = std::move(temp_product);
        }
        return accumulated_product;
    }

	template <typename T>
	size_t CartesianProductCardinality(const std::vector<std::vector<T>>& vec) {
		size_t ret = 0;
		for (const auto& inner : vec) {
			ret *= inner.size();
		}
		return ret;
	}

}