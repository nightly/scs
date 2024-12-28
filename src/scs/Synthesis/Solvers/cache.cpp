#include "cache.h"

#include "ankerl/unordered_dense.h"

#include "scs/SituationCalculus/situation_calculus.h"

namespace scs {

	void SituationActionCache::AddAction(const Situation& s, const CompoundAction& ca) {
		cache_[s] = ca;
	}

}