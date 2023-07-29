#pragma once

#include <ostream>

namespace scs {

	struct RoutePredicate {
		size_t i;
		size_t j;

		RoutePredicate(size_t i, size_t j) : i(i), j(j) {}

		bool operator==(const RoutePredicate& other) const {
			// Order doesn't matter
			return (this->i == other.i && this->j == other.j) ||
				(this->i == other.j && this->j == other.i);
		}

		bool operator!=(const RoutePredicate& other) const {
			return !(*this == other);
		}
	};

	inline std::ostream& operator<< (std::ostream& os, const RoutePredicate& mat) {
		os << "RoutePredicate(" << mat.i << ", " << mat.j << ")";
		return os;
	}

}

