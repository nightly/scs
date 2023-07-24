#include "scs/Synthesis/traces/traces.h"

#include "scs/Synthesis/traces/evolution.h"

namespace scs {
	Traces::Traces(const std::unordered_set<Object>& objects) 
		: objects_(&objects), inst_(objects) {}

	const std::vector<Evolution>& Traces::Get(const std::vector<TransitionType>& transitions) {
		if (cache.contains(transitions)) {

		} else {
			Expand(transitions);
		}
		return cache.at(transitions);
	}

	void Traces::Expand(const std::vector<TransitionType>& transitions) {
		std::vector<Evolution> evolution;

		do {
			Evolution evol;

			evolution.emplace_back(evol);
		} while (std::next_permutation(transitions.begin(), transitions.end()));
	
		
		cache[transitions] = evolution;
	}

}