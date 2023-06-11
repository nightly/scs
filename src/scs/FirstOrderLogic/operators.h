#pragma once

#include <ostream>

#include "scs/FirstOrderLogic/formula.h"
#include "scs/Memory/box.h"
#include "scs/FirstOrderLogic/unary.h"
#include "scs/FirstOrderLogic/binary.h"
#include "scs/FirstOrderLogic/quantifier.h"

#include "scs/FirstOrderLogic/symbols.h"
#include "scs/FirstOrderLogic/printer.h"

namespace scs {

	inline Formula operator!(Formula&& f) {
		return Box<UnaryConnective>{new UnaryConnective{ std::move(f), UnaryKind::Negation }};
	}

	inline Formula operator&&(Formula&& lhs, Formula&& rhs) {
		return Box<BinaryConnective>{new BinaryConnective{ std::move(lhs), std::move(rhs), BinaryKind::Conjunction }};
	}

	inline Formula operator||(Formula&& lhs, Formula&& rhs) {
		return Box<BinaryConnective>{new BinaryConnective{ std::move(lhs), std::move(rhs), BinaryKind::Disjunction }};
	}

	inline std::ostream& operator<<(std::ostream& os, const Formula& f) {
		std::visit(Printer{os, unicode}, f);
		return os;
	}

	inline Formula a_eq(const Action& a) {
		return Box<BinaryConnective>{new BinaryConnective{ scs::Variable{"a"}, a, BinaryKind::Equal }};
	}

	inline Formula cv() {
		return scs::Variable{"cv"};
	}
	
}

// We could also define equality, inequality, implication, equivalence, but these are probably too ambigious/confusing.