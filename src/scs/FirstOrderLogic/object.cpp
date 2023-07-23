#include "scs/FirstOrderLogic/object.h"
#include "scs/SituationCalculus/situation.h"

#include <string_view>

namespace scs {

	void Object::AddObjectToDomain(Situation& s) {
		s.objects.emplace(*this);
	}

	void ObjectUSetPrint(const std::unordered_set<Object>& set, std::ostream& os, std::string_view delimiter, std::string_view indent) {
		for (auto it = set.begin(); it != set.end(); ++it) {
			if (it != set.begin()) {
				os << delimiter;
			}
			os << *it;
		}
	}
}