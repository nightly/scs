#pragma once

#include <vector>
#include <unordered_set>

#include "scs/FirstOrderLogic/object.h"

#include "scs/SituationCalculus/relational_fluent.h"
#include "scs/SituationCalculus/successor.h"
#include "scs/SituationCalculus/situation.h"
#include "scs/SituationCalculus/coop_matrix.h"

namespace scs {
	
	struct Domain {
	public:
		const Situation* situation = nullptr;
		const CoopMatrix* mat = nullptr;
	public:
		Domain() = default;
		
		Domain(const Situation* s) : situation(s) {
		}
		
		Domain(const Situation& s) : situation(&s) {
		}

		Domain(const Situation* s, const CoopMatrix* mat) : situation(s), mat(mat) {
		}

		Domain(const Situation& s, const CoopMatrix& mat) : situation(&s), mat(&mat) {
		}


	};


}