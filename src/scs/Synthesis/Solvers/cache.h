#pragma once

#include "ankerl/unordered_dense.h"

#include "scs/SituationCalculus/situation_calculus.h"

namespace scs {

	struct SituationActionCache {
	public:
		ankerl::unordered_dense::map<Situation, CompoundAction> cache_;
	public:
		
		void AddAction(const Situation& s, const CompoundAction& ca);

		template <typename Container>
		void AddActions(const Situation& s, const Container& cas) {
			for (auto const& ca : cas) {
				AddAction(s, ca);
			}
		}

	};
}