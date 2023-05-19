#pragma once

#include <ostream>

namespace scs {

	struct CoopMatrixPredicate {
		size_t i;
		size_t j;

		CoopMatrixPredicate(size_t i, size_t j) : i(i), j(j) {}

		bool operator==(const CoopMatrixPredicate& other) const {
			// Order doesn't matter
			return (this->i == other.i && this->j == other.j) ||
				(this->i == other.j && this->j == other.i);
		}

		bool operator!=(const CoopMatrixPredicate& other) const {
			!(*this == other);
		}
	};

	inline std::ostream& operator<< (std::ostream& os, const CoopMatrixPredicate& mat) {
		os << "CoopMatrixPredicate(" << mat.i << ", " << mat.j << ")";
		return os;
	}

}

