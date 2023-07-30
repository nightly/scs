#include "utils.h"

namespace scs {

	bool Holds(const Situation& s, const BasicActionTheory& bat, const Formula& f, const FirstOrderAssignment& assignment) {
		scs::Evaluator eval{ {s, bat, bat.CoopMx(), bat.RoutesMx()}, assignment };
		return std::visit(eval, f);
	}

}