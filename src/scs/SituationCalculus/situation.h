#pragma once

#include <vector>
#include <cassert>

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
		std::vector<std::variant<Action, CompoundAction>> history;
		ankerl::unordered_dense::map<std::string, RelationalFluent> relational_fluents_;
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

		bool operator==(const Situation& other) const;
		bool operator!=(const Situation& other) const;
		friend std::ostream& operator<< (std::ostream& stream, const Situation& sit);
	};

	std::ostream& operator<< (std::ostream& os, const std::variant<Action, CompoundAction>& act);
}