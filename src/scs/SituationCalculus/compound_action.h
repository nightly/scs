#pragma once

#include <vector>

#include "scs/SituationCalculus/action.h"

namespace scs {

	struct CompoundAction {
	private:
		std::vector<Action> actions_;
	public:
		CompoundAction() = default;
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

		bool ContainsActionName(std::string_view name) const {
			for (const auto& act : actions_) {
				if (act.name == name) {
					return true;
				}
			}
			return false;
		}


		bool operator==(const CompoundAction& other) const {
			return actions_ == other.actions_;
		}

		bool operator!=(const CompoundAction& other) const {
			return !(other == *this);
		}

		friend std::ostream& operator<<(std::ostream& os, const CompoundAction& ca);
	};

	inline std::ostream& operator<<(std::ostream& os, const CompoundAction& ca) {
		os << "{";
		for (size_t i = 0; i < ca.actions_.size(); ++i) {
			os << ca.actions_[i];
			if (i != ca.actions_.size() - 1) {
				os << ", ";
			}
		}
		os << "}";
		return os;
	}
}