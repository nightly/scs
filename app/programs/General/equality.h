#pragma once

#include <iostream>
#include <format>

#include "scs/SituationCalculus/situation_calculus.h"
#include "scs/Common/strings.h"
#include "scs/FirstOrderLogic/fol.h"

using namespace scs;

inline void EqualityProg() {
	scs::SetConsoleEncoding();
	scs::Situation s;
	scs::BasicActionTheory bat;
	bat.objects.emplace("robot1");
	bat.objects.emplace("robot2");
	scs::Evaluator eval{ {s, bat} };

	scs::Formula f = BinaryConnective(scs::Object{ "robot1" }, scs::Object{ "robot1" }, BinaryKind::Equal);
	auto result = std::visit(eval, f);

	f = BinaryConnective(scs::Object{ "robot1" }, scs::Object{ "robot2" }, BinaryKind::Equal);
	result = std::visit(eval, f);

	f = Quantifier("x", BinaryConnective(scs::Object{ "robot1" }, scs::Variable{ "x" }, BinaryKind::Equal), QuantifierKind::Existential);
	result = std::visit(eval, f);
}