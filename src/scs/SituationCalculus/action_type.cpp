#include "action_type.h"

#include <ostream>

#include <magic_enum.hpp>

namespace scs {

	std::ostream& operator << (std::ostream& os, const ActionType& act) {
		os << magic_enum::enum_name(act);
		return os;
	}

}