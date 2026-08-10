#include "utils.h"

namespace scs {

	bool Holds(const Situation& s, const BasicActionTheory& bat, const Formula& f, const FirstOrderAssignment& assignment) {
		scs::Evaluator eval{{s, bat}, assignment};
		return std::visit(eval, f);
	}

}
