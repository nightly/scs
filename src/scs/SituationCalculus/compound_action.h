#pragma once

#include <vector>

#include "scs/SituationCalculus/action.h"

#include <boost/container_hash/hash.hpp>

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

		template <typename A>
		void AppendAction(A&& simple_act) {
			actions_.emplace_back(std::forward<A>(simple_act));
		}

		const std::vector<Action>& Actions() const { 
			return actions_; 
		}
		void SetActions(std::vector<Action>&& acts) {
			actions_ = std::move(acts);
		}
		void SetActions(const std::vector<Action>& acts) {
			actions_ = acts;;
		}
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

namespace std {

	template <>
	struct std::hash<scs::CompoundAction> {
		size_t operator() (const scs::CompoundAction& ca) const {
			size_t seed = 0;
			for (const auto& act : ca.Actions()) {
				boost::hash_combine(seed, std::hash<scs::Action>()(act));
			}
			return seed;
		}
	};

}