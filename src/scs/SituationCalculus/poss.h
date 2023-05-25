#pragma once

#include "scs/SituationCalculus/action.h"
#include "scs/FirstOrderLogic/formula.h"


namespace scs {

	struct Poss {
	private:
		// We assume all Poss are situation suppressed
		Formula formula_;
		std::vector<Term> terms_;

	public:
		Poss() : formula_(true) {}
		Poss(const Formula& f);
		Poss(Formula&& f);
		Poss(const std::vector<Term>& terms, const Formula& f);
		Poss(std::vector<Term>&& terms, Formula&& f);

		const Formula& Form() const;
		const std::vector<Term>& Terms() const;
	};

}