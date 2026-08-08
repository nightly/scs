#include "scs/SituationCalculus/object_universe.h"

#include <stdexcept>

#include "scs/SituationCalculus/action.h"
#include "scs/SituationCalculus/bat.h"
#include "scs/SituationCalculus/compound_action.h"
#include "scs/SituationCalculus/situation.h"
#include "scs/FirstOrderLogic/assignment.h"

namespace scs {

	ObjectSet RelevantObjects(const Situation& situation, const BasicActionTheory& bat) {
		ObjectSet objects = bat.objects;
		for (const auto& [name, fluent] : situation.Fluents()) {
			(void)name;
			for (const auto& tuple : fluent.TrueTuples()) {
				objects.insert(tuple.begin(), tuple.end());
			}
		}
		return objects;
	}

	void AddGroundActionObjects(ObjectSet& objects, const Action& action) {
		for (const auto& term : action.terms) {
			const auto* object = std::get_if<Object>(&term);
			if (object == nullptr) {
				throw std::invalid_argument("Action '" + action.name + "' is not ground");
			}
			objects.emplace(*object);
		}
	}

	void AddGroundActionObjects(ObjectSet& objects, const CompoundAction& action) {
		for (const auto& component : action.Actions()) {
			AddGroundActionObjects(objects, component);
		}
	}

	void AddAssignmentObjects(ObjectSet& objects, const FirstOrderAssignment& assignment) {
		for (const auto& [variable, value] : assignment) {
			(void)variable;
			if (const auto* object = std::get_if<Object>(&value)) {
				objects.emplace(*object);
			}
		}
	}

	ObjectSet RelevantObjects(const Situation& situation, const BasicActionTheory& bat, const Action& action) {
		auto objects = RelevantObjects(situation, bat);
		AddGroundActionObjects(objects, action);
		return objects;
	}

	ObjectSet RelevantObjects(const Situation& situation, const BasicActionTheory& bat, const CompoundAction& action) {
		auto objects = RelevantObjects(situation, bat);
		AddGroundActionObjects(objects, action);
		return objects;
	}

}
