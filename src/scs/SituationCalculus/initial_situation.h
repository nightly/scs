#pragma once

#include <vector>

#include "scs/SituationCalculus/situation.h"

namespace scs {

	/*
	* Distinguished intiial situation, S_0.
	* No actions performed.
	*/
	struct InitialSituation {
	private:
		Situation situation_;
	public:
		InitialSituation() = delete;
		InitialSituation(const Situation& s);
		InitialSituation(Situation&& s);

		const Situation& situation() const;
	};

}