#pragma once

#include <span>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "scs/SituationCalculus/successor.h"
#include "scs/SituationCalculus/action.h"
#include "scs/SituationCalculus/poss.h"
#include "scs/SituationCalculus/coop_matrix.h"

namespace scs {

	struct BasicActionTheory {
	public:
		std::unordered_map<std::string, Poss> pre;
		std::unordered_map<std::string, Successor> successors;

		bool is_global;
	private:
		Situation initial_; // Encapsulates initial situation description
		CoopMatrix mat_;
	public:
		const Situation& Initial() const {
			return initial_;
		}

		const CoopMatrix& CoopMx() const {
			return mat_;
		}

		template <typename S>
		void SetInitial(S&& initial_situation) {
			assert(initial_situation.history.size() == 0 && "Initial situation cannot have a history of actions");
			initial_ = std::forward<S>(initial_situation);
		}

		template <typename C>
		void SetCoopMx(C&& CoopMatrix) {
			assert(is_global && "CoopMatrix being set for a non-global BAT instead of being set at endpoint");
			mat_ = std::forward<C>(mat_);
		}

	};

}

// 		std::unordered_set<Action> actions;