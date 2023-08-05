#pragma once

#include <ostream>

namespace scs {

	enum class ActionType {
		Manufacturing = 0,
		Nop = 1,
		Transfer = 2,
		Prepatory = 2,
	};

	inline std::ostream& operator << (std::ostream& os, const ActionType& act);

}