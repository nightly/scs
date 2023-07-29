#pragma once

#include <ostream>

#include "scs/FirstOrderLogic/variable.h"

namespace scs {

	struct CoopPredicate {
		scs::Variable i;
		scs::Variable j;

		CoopPredicate(scs::Variable i, scs::Variable j) : i(i), j(j) {}

		bool operator==(const CoopPredicate& other) const {
			// Order doesn't matter
			return (this->i == other.i && this->j == other.j) ||
				(this->i == other.j && this->j == other.i);
		}

		bool operator!=(const CoopPredicate& other) const {
			return !(*this == other);
		}
	};

	inline std::ostream& operator<< (std::ostream& os, const CoopPredicate& mat) {
		os << "CoopMatrixPredicate(" << mat.i << ", " << mat.j << ")";
		return os;
	}

}

