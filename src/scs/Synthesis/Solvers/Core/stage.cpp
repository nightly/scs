#include "stage.h"

#include <string>
#include "magic_enum.hpp"

namespace scs {

	std::ostream& operator<< (std::ostream& os, const Stage& stage) {
		std::string indent_space(1, ' ');
		os << "Stage:" << "\n";
		os << indent_space << "Type: " << magic_enum::enum_name(stage.type) << "\n";
		os << indent_space << "Label: " << stage.recipe_transition.label() << "\n";
		os << indent_space << "To: " << stage.recipe_transition.to() << "\n";
		os << indent_space << "Situation: \n";
		os << indent_space;
		stage.sit.PrintHistory(os);
		os << "\n";

		return os;
	}

}