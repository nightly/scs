#pragma once

#include <unordered_set>
#include <vector>
#include <assert.h>

namespace scs {

	// Built-in predicate for cooperation between resources.
	// This class stores all the information for CoopMatrix.
	struct CoopMatrix {
	private:
		std::vector<std::unordered_set<size_t>> vec_;
	public:
		CoopMatrix(size_t num_resources) {
			vec_.resize(num_resources);
		}

		void Add(size_t i, size_t j) {
			vec_[i].emplace(j);
			vec_[j].emplace(i);
		}

		bool Lookup(size_t i, size_t j) const {
			if (i >= vec_.size() || j>= vec_.size()) {
				assert(true && "CoopMatrix index out of bounds");
				return false;
			}

			return vec_[i].contains(j);
		}

	};

}