#pragma once

#include <iostream>
#include <format>

#include "scs/SituationCalculus/situation_calculus.h"
#include "scs/Common/strings.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/ConGolog/Program/programs.h"
#include "scs/ConGolog/resource.h"

using namespace scs;

struct ExampleCommon {
public:
	Formula wr1 = BinaryConnective(scs::Variable{"j"}, scs::Variable{"i"}, BinaryKind::NotEqual);
	Formula wr2 = Predicate{ "at", {scs::Variable{"part"}, scs::Variable{"j"}} };
	Formula wr3 = CoopPredicate(scs::Variable{"i"}, scs::Variable{"j"});
	Formula within_reach = Quantifier("j", BinaryConnective(BinaryConnective(wr1, wr2, BinaryKind::Conjunction), wr3, BinaryKind::Conjunction),
		QuantifierKind::Existential);

	Formula Nop_form = true;

};

inline BasicActionTheory ExampleCommonBAT() {
	BasicActionTheory ret;

	ret.pre["Nop"] = { {}, true };

	return ret;
}