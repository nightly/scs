#pragma once

#include <vector>
#include <unordered_set>

#include "scs/FirstOrderLogic/object.h"

#include "scs/SituationCalculus/relational_fluent.h"
#include "scs/SituationCalculus/successor.h"
#include "scs/SituationCalculus/situation.h"

namespace scs {

	/***
	* For now, we embed the situation to be the domain, containing the fluents and objects.
	* Since this results in a lot of small object copies which may add up,
	* it may be more efficient to make objects and fluents stored independently of situations,
	* with a lookup present for each situation to the object/fluent. Though it's easier to debug
	* this way.
	***/
	
	struct Domain {
	public:
		const Situation* situation = nullptr;
	public:
		Domain() = default;
		Domain(const Situation* s) : situation(s) {}
	};


}