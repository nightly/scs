#pragma once

#include <vector>

namespace scs {


	template <typename T>
    std::vector<std::vector<T>> Product(std::vector<const std::vector<T>*> input_vectors, T default_value = T()) {
        // Handle any vectors that are empty by replacing with a vector of 1 default value
        std::vector<T> default_vec = { default_value };
        for (auto& vec : input_vectors) {
            if (vec->empty()) {
                vec = &default_vec;
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
    inline std::ostream& operator<< (std::ostream& os, const std::vector<std::vector<T>>& vec) {
        for (const auto& inner : vec) {
            for (size_t i = 0; i < inner.size(); ++i) {
                os << inner[i];
                if (i != inner.size() - 1) {
                    os << ", ";
                }
            }
            os << "\n";
        }
        return os;
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