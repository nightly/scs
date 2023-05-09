#pragma once

#include <vector>

#include "scs/SituationCalculus/action.h"

namespace scs {

	struct CompoundAction {
	private:
		std::vector<Action> actions_;
	public:
		CompoundAction(const std::vector<Action>& actions) : actions_(actions) {}
		CompoundAction(std::vector<Action>&& actions) : actions_(std::move(actions)) {}

		CompoundAction(const Action& act) {
			actions_.emplace_back(act);
		}
		CompoundAction(Action&& act) {
			actions_.emplace_back(std::move(act));
		}

		const std::vector<Action>& Actions() const { return actions_; }

		bool IsSimple() const {
			return actions_.size() == 1;
		}
	};

}