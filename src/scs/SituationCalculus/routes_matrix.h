#pragma once

#include <unordered_set>
#include <vector>
#include <assert.h>

namespace scs {

	// Situation independent predicate for routes between resources.
	struct RoutesMatrix {
	private:
		std::vector<std::unordered_set<size_t>> vec_;
	public:
		RoutesMatrix() = default;

		RoutesMatrix(size_t num_resources) {
			vec_.resize(num_resources);
		}

		const auto& vec() const { return vec_; }

		void Add(size_t i, size_t j) {
			vec_[i].emplace(j);
			vec_[j].emplace(i);
		}

		// Order of lookup's (i, j) should not matter due to references being added to both resources.
		bool Lookup(size_t i, size_t j) const {
			if (i >= vec_.size() || j >= vec_.size()) {
				assert(true && "Routable index out of bounds");
				return false;
			}

			return vec_[i].contains(j);
		}
	private:
		friend std::ostream& operator<<(std::ostream& os, const RoutesMatrix& routes);
	};

	inline std::ostream& operator<< (std::ostream& os, const RoutesMatrix& routes) {
		os << "[";
		for (size_t i = 0; i < routes.vec_.size(); ++i) {
			for (const auto& j : routes.vec_[i]) {
				os << "{" << i << "," << j << "}" << " ";
			}
		}
		os << " ]";
		return os;
	}

}