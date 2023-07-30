#pragma once

#include <vector>

#include "scs/SituationCalculus/action.h"

#include <ankerl/unordered_dense.h>
#include <boost/container_hash/hash.hpp>

namespace scs {

	struct CompoundAction {
	private:
		std::vector<Action> actions_;
	public:
		CompoundAction() = default;
		CompoundAction(const std::vector<Action>& actions);
		CompoundAction(std::vector<Action>&& actions);
		CompoundAction(const Action& act);
		CompoundAction(Action&& act);

		template <typename A>
		void AppendAction(A&& simple_act) {
			actions_.emplace_back(std::forward<A>(simple_act));
		}

		const std::vector<Action>& Actions() const;
		void SetActions(std::vector<Action>&& acts);
		void SetActions(const std::vector<Action>& acts);
		bool IsSimple() const;

		ankerl::unordered_dense::set<std::string> NamedActions() const;

		bool ContainsActionName(std::string_view name) const;

		bool operator==(const CompoundAction& other) const;
		bool operator!=(const CompoundAction& other) const;

		friend std::ostream& operator<<(std::ostream& os, const CompoundAction& ca);
	};

	std::ostream& operator<<(std::ostream& os, const CompoundAction& ca);
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