#pragma once

#include <vector>
#include <cassert>

#include "scs/SituationCalculus/action.h"
#include "scs/SituationCalculus/action_state.h"
#include "scs/SituationCalculus/relational_fluent.h"
#include "scs/FirstOrderLogic/object.h"

namespace scs {

	class Poss;
	class Successor;

	struct Situation {
	public:
		std::unordered_set<Object> objects;
		std::vector<Action> history;
		std::unordered_map<std::string, RelationalFluent> relational_fluents_;
	public:
		void AddFluent(const RelationalFluent& fluent);
		void AddFluent(RelationalFluent&& fluent);

		bool Possible(const Action& a, const Poss& pre) const;
		bool Possible(const ActionState& a, const Poss& pre) const;
		Situation Do(const Action& a, const Poss& pre, const Successor& ssa) const;
		Situation Do(const ActionState& a, const Poss& pre, const Successor& ssa) const;

		size_t Length() const;

		void PrintObjects(std::ostream& output_stream = std::cout) const;
		void PrintFluents(std::ostream& output_stream = std::cout) const;

		bool operator==(const Situation& other) const;
		bool operator!=(const Situation& other) const;
		friend std::ostream& operator<< (std::ostream& stream, const Situation& sit);
	};

}