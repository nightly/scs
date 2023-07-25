#pragma once

#include <vector>
#include <ostream>
#include <fstream>

#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"

namespace nightly {

	template <>
	inline void WriteXLabel(const std::vector<scs::CgState>& state_vec, std::ostream& os) {
		os << "<[";
		for (size_t i = 0; i < state_vec.size(); ++i) {
			os << state_vec[i].final_condition;
			if (i != state_vec.size() - 1) {
				os << ", ";
			}
		}
		os << "]>";
	}

}