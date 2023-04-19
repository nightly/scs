#pragma once

#include <string>
#include <unordered_set>
#include <vector>

#include "scs/FirstOrderLogic/object.h"

namespace scs {

	struct FunctionalFluent {
	public:
		std::string name;
		const std::function<Object(std::initializer_list<Object> parameters)>* func; // Takes objects, produces an an object

	};

}