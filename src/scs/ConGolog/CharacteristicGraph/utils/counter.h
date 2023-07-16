#pragma once

#include <vector>
#include <optional>
#include <ostream>

namespace scs {

	struct StateCounter {
	private:
		size_t n = 0;
	public:
		StateCounter() : n(0) {}

		size_t Increment() {
			n++;
			return n;
		}

		friend std::ostream& operator<< (std::ostream& os, const StateCounter& sc);
	};


	inline std::ostream& operator<< (std::ostream& os, const StateCounter& sc) {
		os << "Counter: ";
		os << sc.n;
		return os;
	}
}