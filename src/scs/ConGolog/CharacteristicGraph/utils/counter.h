#pragma once

#include <vector>
#include <optional>

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
	};

}