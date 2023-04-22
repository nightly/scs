#pragma once

#include <span>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "scs/SituationCalculus/successor.h"
#include "scs/SituationCalculus/poss.h"

namespace scs {

	struct BasicActionTheory {
		Situation initial; // Encapsulates initial situation description
		std::unordered_map<std::string, Poss> pre;
		std::unordered_map<std::string, Successor> successors;
	};

}