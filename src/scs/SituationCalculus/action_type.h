#pragma once

#include <ostream>

namespace scs {

	enum class ActionType {
		Nop = 1,
		Transfer = 2,
		Prepatory = 2,
		Manufacturing = 4,
	};

	inline std::ostream& operator << (std::ostream& os, const ActionType& act);

}