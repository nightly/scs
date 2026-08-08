#pragma once

#include "ankerl/unordered_dense.h"

#include "scs/FirstOrderLogic/object.h"

namespace scs {

	class BasicActionTheory;
	struct Action;
	struct CompoundAction;
	struct Situation;

	using ObjectSet = ankerl::unordered_dense::set<Object>;

	ObjectSet RelevantObjects(const Situation& situation, const BasicActionTheory& bat);
	ObjectSet RelevantObjects(const Situation& situation, const BasicActionTheory& bat, const Action& action);
	ObjectSet RelevantObjects(const Situation& situation, const BasicActionTheory& bat, const CompoundAction& action);
	void AddGroundActionObjects(ObjectSet& objects, const Action& action);
	void AddGroundActionObjects(ObjectSet& objects, const CompoundAction& action);

}
