#include "cache.h"

#include "ankerl/unordered_dense.h"

#include "scs/SituationCalculus/situation_calculus.h"

namespace scs {

	void SituationCache::AddAction(const Situation& s, const CompoundAction& ca) {
		cache_[s] = ca;
	}

	bool SituationCache::Exists(const Situation& s) {
		return cache_.contains(s);
	}

}