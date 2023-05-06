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
		std::unordered_map<std::string, Poss> pre;
		std::unordered_map<std::string, Successor> successors;
	private:
		Situation initial_; // Encapsulates initial situation description
	public:

		const Situation& Initial() const {
			return initial_;
		}

		template <typename S>
		void SetInitial(S&& initial_situation) {
			assert(initial_situation.history.size() == 0 && "Initial situation cannot have a history of actions");
			initial_ = std::forward<S>(initial_situation);
		}

	};

}