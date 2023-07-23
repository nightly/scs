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
		CoopMatrix() = default;

		CoopMatrix(size_t num_resources) {
			vec_.resize(num_resources);
		}

		const auto& vec() const { return vec_; }

		void Add(size_t i, size_t j) {
			vec_[i].emplace(j);
			vec_[j].emplace(i);
		}

		// Order of lookup's (i, j) should not matter due to references being added to both resources.
		bool Lookup(size_t i, size_t j) const {
			if (i >= vec_.size() || j>= vec_.size()) {
				assert(true && "CoopMatrix index out of bounds");
				return false;
			}

			return vec_[i].contains(j);
		}

		bool IsEmpty() const {
			for (const auto& v : vec_) {
				if (!v.empty()) {
					return false;
				}
			}
			return true;
		}
	private:
		friend std::ostream& operator<<(std::ostream& os, const CoopMatrix& mat);
	};

	inline std::ostream& operator<< (std::ostream& os, const CoopMatrix& mat) {
		os << "[";
		for (size_t i = 0; i < mat.vec_.size(); ++i) {
			for (const auto& j : mat.vec_[i]) {
				os << "{" << i << "," << j << "}" << " ";
			}
		}
		os << " ]";
		return os;
	}

}