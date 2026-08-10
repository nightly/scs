#include "scs/FirstOrderLogic/domain.h"

#include "scs/SituationCalculus/bat.h"

namespace scs {

	const Domain::ObjectSet& Domain::Objects() const {
		if (object_universe != nullptr) return *object_universe;
		if (bat != nullptr) return bat->objects;
		static const ObjectSet empty;
		return empty;
	}

	Domain Domain::WithAnonymousIdentifiers(size_t count) const {
		if (semantics != DomainSemantics::InfiniteGeneric || count == 0) {
			return *this;
		}

		Domain result = *this;
		result.owned_object_universe = std::make_shared<ObjectSet>(Objects());
		for (size_t i = 0, added = 0; added < count; ++i) {
			if (result.owned_object_universe->emplace(
				Object::Identifier("@scs-anonymous-" + std::to_string(i))).second) {
				++added;
			}
		}
		result.object_universe = result.owned_object_universe.get();
		return result;
	}

}
