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
	public:
		Situation initial; // Encapsulates initial situation description
		std::unordered_map<std::string, Poss> pre;
		std::unordered_map<std::string, Successor> successors;

	public:
		void SetInitial(const Situation& s) {
			initial = s;
			assert(s.history.size() == 0 && "Initial situation cannot have a history of actions");
		}

		void SetInitial(Situation&& s) {
			initial = std::move(s);
			assert(s.history.size() == 0 && "Initial situation cannot have a history of actions");
		}


	};

}