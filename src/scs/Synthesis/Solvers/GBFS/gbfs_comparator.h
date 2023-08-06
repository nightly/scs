#pragma once

#include "scs/Synthesis/Solvers/Core/candidate.h"

namespace scs {

	// Max order greedy comparator
	// i.e. order by picking the candidate that has completed the most recipe transitions so far
	struct GreedyCandidateComparator {
	public:
		GreedyCandidateComparator() {}

		bool operator () (const Candidate& a, const Candidate& b) {
			return (a.completed_recipe_transitions) < (b.completed_recipe_transitions);
		}
	};


}