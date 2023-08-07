#pragma once

#include "scs/Synthesis/Solvers/Core/candidate.h"

namespace scs {

	// Max order (pq-wise) greedy comparator
	// i.e. order by picking the candidate that has completed the most recipe transitions so far and continue that
	struct GreedyCandidateComparator {
	public:
		GreedyCandidateComparator() {}

		bool operator () (const Candidate& a, const Candidate& b) {
			return (a.completed_recipe_transitions + a.total_transitions) < (b.completed_recipe_transitions + b.total_transitions);
		}
	};


}