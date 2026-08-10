#pragma once

#include "ankerl/unordered_dense.h"

#include "scs/FirstOrderLogic/object.h"

namespace scs {

	class BasicActionTheory;
	struct FirstOrderAssignment;
	struct Action;
	struct CompoundAction;
	struct Situation;

	using ObjectSet = ankerl::unordered_dense::set<Object>;

	ObjectSet RelevantObjects(const Situation& situation, const BasicActionTheory& bat);
	ObjectSet RelevantObjects(const Situation& situation, const BasicActionTheory& bat, const Action& action);
	ObjectSet RelevantObjects(const Situation& situation, const BasicActionTheory& bat, const CompoundAction& action);
	ObjectSet ActiveIdentifiers(const Situation& situation);
	size_t RenameableActiveDomainSize(const Situation& situation);
	void AddGroundActionObjects(ObjectSet& objects, const Action& action);
	void AddGroundActionObjects(ObjectSet& objects, const CompoundAction& action);
	void AddAssignmentObjects(ObjectSet& objects, const FirstOrderAssignment& assignment);

}
