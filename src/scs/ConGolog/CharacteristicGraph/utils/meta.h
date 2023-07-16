#pragma once

#include <vector>
#include <optional>
#include <ostream>

namespace scs {

	struct StateMeta {
	public:
		std::optional<std::vector<size_t>> loop_back = std::nullopt;

	public:

		friend std::ostream& operator<< (std::ostream& os, const StateMeta& sm);
	};


	inline std::ostream& operator<< (std::ostream& os, const StateMeta& sm) {
		os << "Meta: ";
		if (sm.loop_back.has_value()) {
			os << "Loop back: ";
			for (size_t i = 0; i < (*sm.loop_back).size(); ++i) {
				os << (*sm.loop_back).at(i);
				if (i != (*sm.loop_back).size() - 1) {
					os << ",";
				}
			}
			os << ".\n";
		}
		return os;
	}

}