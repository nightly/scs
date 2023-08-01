#include "scs/SituationCalculus/compound_action.h"

namespace scs {

	CompoundAction::CompoundAction(const std::vector<Action>& actions) : actions_(actions) {}
	CompoundAction::CompoundAction(std::vector<Action>&& actions) : actions_(std::move(actions)) {}
	CompoundAction::CompoundAction(const Action& act) {
		actions_.emplace_back(act);
	}
	CompoundAction::CompoundAction(Action&& act) {
		actions_.emplace_back(std::move(act));
	}

	const std::vector<Action>& CompoundAction::Actions() const {
		return actions_;
	}
	void CompoundAction::SetActions(std::vector<Action>&& acts) {
		actions_ = std::move(acts);
	}
	void CompoundAction::SetActions(const std::vector<Action>& acts) {
		actions_ = acts;;
	}
	bool CompoundAction::IsSimple() const {
		return actions_.size() == 1;
	}

	ankerl::unordered_dense::set<std::string> CompoundAction::NamedActions() const {
		ankerl::unordered_dense::set<std::string> ret;
		ret.reserve(actions_.size());
		for (const auto& act : actions_) {
			ret.emplace(act.name);
		}
		return ret;
	}

	bool CompoundAction::ContainsActionName(std::string_view name) const {
		for (const auto& act : actions_) {
			if (act.name == name) {
				return true;
			}
		}
		return false;
	}

	bool CompoundAction::AreAllNop() const {
		for (const auto& a : actions_) {
			if (a.name != "Nop") {
				return false;
			}
		}
		return true;
	}

	bool CompoundAction::operator==(const CompoundAction& other) const {
		return (actions_ == other.actions_);
	}

	bool CompoundAction::operator!=(const CompoundAction& other) const {
		return !(other == *this);
	}

	std::ostream& operator<<(std::ostream& os, const CompoundAction& ca) {
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