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

		const std::vector<Action>& Actions() const { return actions_; }
		bool IsSimple() const {
			return actions_.size() == 1;
		}
	};



	inline std::ostream& operator<< (std::ostream& stream, const CompoundAction& ca) {
		stream << "{";
		for (const auto& act : ca.Actions()) {
			stream << act;
		}
		stream << "}";
		return stream;
	}

}