#pragma once

#include <vector>
#include <cassert>

#include "scs/SituationCalculus/situation.h"
#include "scs/SituationCalculus/initial_situation.h"

namespace scs {


	InitialSituation::InitialSituation(const Situation& s) : situation_(s) {
		assert(s.history.size() == 0 && "Initial situation cannot have a history of actions");
	}

	InitialSituation::InitialSituation(Situation&& s) : situation_(std::move(s)) {
		assert(s.history.size() == 0 && "Initial situation cannot have a history of actions");
	}

	const Situation& InitialSituation::situation() const { return situation_; }

}