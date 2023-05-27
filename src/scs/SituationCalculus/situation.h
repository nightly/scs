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
		std::unordered_set<Object> objects;
		std::vector<Action> history;
		std::unordered_map<std::string, RelationalFluent> relational_fluents_;
	public:
		void AddFluent(const RelationalFluent& fluent);
		void AddFluent(RelationalFluent&& fluent);

		bool Possible(const Action& a, const BasicActionTheory& bat) const;
		bool Possible(const CompoundAction& a, const BasicActionTheory& bat) const;
		Situation Do(const Action& a, const BasicActionTheory& bat) const;
		Situation Do(const CompoundAction& a, const BasicActionTheory& bat) const;

		bool ObjectInDomain(const Object& o) const;

		size_t Length() const;

		void PrintHistory(std::ostream& output_stream = std::cout) const;
		void PrintObjects(std::ostream& output_stream = std::cout) const;
		void PrintFluents(std::ostream& output_stream = std::cout) const;

		bool operator==(const Situation& other) const;
		bool operator!=(const Situation& other) const;
		friend std::ostream& operator<< (std::ostream& stream, const Situation& sit);
	};

}