#include "scs/FirstOrderLogic/object.h"
#include "scs/SituationCalculus/situation.h"

namespace scs {

	void Object::AddObjectToDomain(Situation& s) {
		s.objects.emplace(*this);
	}

	void ObjectUSetPrint(const std::unordered_set<Object>& set, char delimiter, std::ostream& os) {
		for (auto it = set.begin(); it != set.end(); ++it) {
			if (it != set.begin()) {
				os << delimiter;
			}
			os << *it;
		}
	}
}