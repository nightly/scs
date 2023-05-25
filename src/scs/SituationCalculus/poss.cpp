#include "scs/SituationCalculus/poss.h"

namespace scs {

	Poss::Poss(const Formula& f) : formula_(f) {}
	Poss::Poss(Formula&& f) : formula_(std::move(f)) {}
	Poss::Poss(const std::vector<Term>& terms, const Formula& f) : terms_(terms), formula_(f) {}
	Poss::Poss(std::vector<Term>&& terms, Formula&& f) : terms_(std::move(terms)), formula_(std::move(f)) {}

	const Formula& Poss::Form() const {
		return formula_;
	}
	const std::vector<Term>& Poss::Terms() const {
		return terms_;
	}

}