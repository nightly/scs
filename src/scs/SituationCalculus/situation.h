#pragma once

#include <vector>
#include <cassert>
#include <ostream>
#include <iostream>

#include <boost/container_hash/hash.hpp>
#include <boost/functional/hash.hpp>

#include "scs/SituationCalculus/action.h"
#include "scs/SituationCalculus/compound_action.h"
#include "scs/SituationCalculus/relational_fluent.h"
#include "scs/FirstOrderLogic/object.h"

namespace scs {

	class Poss;
	class Successor;
	class BasicActionTheory;

	struct Situation {
	public:
		template <typename Key, typename Value>
		using u_map = ankerl::unordered_dense::map<Key, Value>;
	public:
		std::vector<std::variant<Action, CompoundAction>> history;
		u_map<std::string, RelationalFluent> relational_fluents_;
	public:
		void AddFluent(const std::string& name, const RelationalFluent& fluent);
		void AddFluent(const std::string& name, RelationalFluent&& fluent);

		bool Possible(const Action& a, const BasicActionTheory& bat) const;
		bool Possible(const CompoundAction& ca, const BasicActionTheory& bat) const;
		
		Situation Do(const Action& a, const BasicActionTheory& bat) const;
		Situation Do(const CompoundAction& a, const BasicActionTheory& bat) const;

		bool ObjectInDomain(const Object& o, const BasicActionTheory& bat) const;

		size_t Length() const;

		void PrintHistory(std::ostream& output_stream = std::cout) const;
		void PrintFluents(std::ostream& output_stream = std::cout, bool with_history = false, size_t indent = 0) const;

		const u_map<std::string, RelationalFluent>& Fluents() const;

		bool operator==(const Situation& other) const;
		bool operator!=(const Situation& other) const;
		friend std::ostream& operator<< (std::ostream& stream, const Situation& sit);
	};

	std::ostream& operator<< (std::ostream& os, const std::variant<Action, CompoundAction>& act);
}

namespace std {

	template <>
	struct std::hash<scs::Situation> {
		size_t operator() (const scs::Situation& sit) const {
			size_t seed = 0;
			for (const auto& var : sit.history) {
				std::visit([&](const auto& a) {
					boost::hash_combine(seed, std::hash<std::decay_t<decltype(a)>>{}(a));
				}, var);
			}
			return seed;
		}
	};
}