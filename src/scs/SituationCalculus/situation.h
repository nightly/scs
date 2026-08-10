#pragma once

#include <vector>
#include <cassert>
#include <ostream>
#include <iostream>

#include <boost/container_hash/hash.hpp>
#include <boost/functional/hash.hpp>

#include <spdlog/fmt/ostr.h>

#include "scs/SituationCalculus/action.h"
#include "scs/SituationCalculus/compound_action.h"
#include "scs/SituationCalculus/relational_fluent.h"
#include "scs/FirstOrderLogic/object.h"
#include "scs/SituationCalculus/object_universe.h"

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
		bool Possible(const Action& a, const BasicActionTheory& bat, const ObjectSet& objects) const;
		bool Possible(const CompoundAction& ca, const BasicActionTheory& bat, const ObjectSet& objects) const;
		
		Situation Do(const Action& a, const BasicActionTheory& bat, bool markovian_situations = false) const;
		Situation Do(const CompoundAction& a, const BasicActionTheory& bat, bool markovian_situations = false) const;
		Situation Do(const Action& a, const BasicActionTheory& bat, const ObjectSet& objects,
			bool markovian_situations = false) const;
		Situation Do(const CompoundAction& a, const BasicActionTheory& bat, const ObjectSet& objects,
			bool markovian_situations = false) const;

		bool ObjectInDomain(const Object& o, const BasicActionTheory& bat) const;

		size_t Length() const;

		void PrintHistory(std::ostream& output_stream = std::cout) const;
		void PrintFluents(std::ostream& output_stream = std::cout, bool with_history = false, size_t indent = 0) const;

		const u_map<std::string, RelationalFluent>& Fluents() const;

		bool operator==(const Situation& other) const;
		bool operator!=(const Situation& other) const;
		friend std::ostream& operator<< (std::ostream& stream, const Situation& sit);
	};

	using Interpretation = Situation;

	std::ostream& operator<< (std::ostream& os, const std::variant<Action, CompoundAction>& act);
}

template <>
struct fmt::formatter<scs::Situation> : fmt::ostream_formatter {};

namespace std {

	template <>
	struct hash<scs::Situation> {
		size_t operator() (const scs::Situation& sit) const {
			size_t seed = 0;
			for (const auto& [name, fluent] : sit.Fluents()) {
				size_t entry = 0;
				boost::hash_combine(entry, name);
				boost::hash_combine(entry, std::hash<scs::RelationalFluent>{}(fluent));
				seed += entry;
			}
			return seed;
		}
	};
}
