#pragma once

#include <vector>
#include <unordered_set>
#include <memory>

#include "ankerl/unordered_dense.h"

#include "scs/FirstOrderLogic/object.h"

#include "scs/SituationCalculus/relational_fluent.h"
#include "scs/SituationCalculus/successor.h"
#include "scs/SituationCalculus/situation.h"

namespace scs {
	
	struct Domain {
	public:
		using ObjectSet = ankerl::unordered_dense::set<Object>;

		const Situation* situation = nullptr;
		const BasicActionTheory* bat = nullptr;
		const ObjectSet* object_universe = nullptr;
		DomainSemantics semantics = DomainSemantics::Finite;
		std::shared_ptr<ObjectSet> owned_object_universe;
	public:
		Domain() = default;
		
		Domain(const Situation* s, const BasicActionTheory* bat) : situation(s), bat(bat) {
		}
		
		Domain(const Situation& s, const BasicActionTheory& bat) : situation(&s), bat(&bat) {
		}

		Domain(const Situation& s, const BasicActionTheory& bat, const ObjectSet& objects)
			: situation(&s), bat(&bat), object_universe(&objects) {
		}

		const ObjectSet& Objects() const;
		Domain WithAnonymousIdentifiers(size_t count) const;

	};


}
