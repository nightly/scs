#include "scs/FirstOrderLogic/object.h"
#include "scs/SituationCalculus/situation.h"
#include "scs/SituationCalculus/bat.h"

#include <string_view>

#include "ankerl/unordered_dense.h"

namespace scs {

	void Object::AddObjectToDomain(BasicActionTheory& bat) {
		bat.objects.emplace(*this);
	}

	void ObjectUSetPrint(const ankerl::unordered_dense::set<Object>& set, std::ostream& os, std::string_view delimiter, std::string_view indent) {
		for (auto it = set.begin(); it != set.end(); ++it) {
			if (it != set.begin()) {
				os << delimiter;
			}
			os << *it;
		}
	}
}