#include "scs/SituationCalculus/poss.h"

namespace scs {

	Poss::Poss(const Formula& f) : formula_(f) {}
	Poss::Poss(Formula&& f) : formula_(std::move(f)) {}
	Poss::Poss(const Formula& f, const std::vector<Term>& terms) : formula_(f), terms_(terms) {}
	Poss::Poss(Formula&& f, std::vector<Term>&& terms) : formula_(std::move(f)), terms_(std::move(terms)) {}

	const Formula& Poss::Form() const {
		return formula_;
	}
	const std::vector<Term>& Poss::Terms() const {
		return terms_;
	}

}