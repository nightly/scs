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
	Situation s0;

	// Objects & initial valuations
	s0.objects.emplace("brass");
	s0.objects.emplace("tube");
	s0.objects.emplace("metallic_red");
	s0.objects.emplace("metallic_blue");

	s0.relational_fluents_["on_site"].AddValuation({ Object{"brass"} }, true);
	s0.relational_fluents_["part"].AddValuation({ Object{"brass"} }, true);
	s0.relational_fluents_["clamped"].AddValuation({ Object{"brass"} }, false);
	s0.relational_fluents_["material"].AddValuation({ Object{"brass"} }, true);
	s0.relational_fluents_["suitable"].AddValuation({ Object{"5mm"}, Object{"0.5"}}, true);

	s0.relational_fluents_["on_site"].AddValuation({ Object{"tube"} }, true);
	s0.relational_fluents_["part"].AddValuation({ Object{"tube"} }, true);
	s0.relational_fluents_["clamped"].AddValuation({ Object{"tube"} }, false);

	s0.relational_fluents_["equipped_bit"].AddValuation({Object{ "3mm" }, Object{ "3" }}, false);
	s0.relational_fluents_["equipped_bit"].AddValuation({Object{ "5mm" }, Object{ "3" }}, false);

	s0.relational_fluents_["at"].AddValuation({Object{ "brass"}, Object{"1"} }, false);
	s0.relational_fluents_["at"].AddValuation({Object{ "brass"}, Object{"2"} }, false);
	s0.relational_fluents_["at"].AddValuation({Object{ "brass"}, Object{"3"} }, false);
	s0.relational_fluents_["at"].AddValuation({Object{ "brass"}, Object{"4"} }, false);
	s0.relational_fluents_["at"].AddValuation({Object{ "tube"}, Object{"1"} }, false);
	s0.relational_fluents_["at"].AddValuation({Object{ "tube"}, Object{"2"} }, false);
	s0.relational_fluents_["at"].AddValuation({Object{ "tube"}, Object{"3"} }, false);
	s0.relational_fluents_["at"].AddValuation({Object{ "tube"}, Object{"4"} }, false);

	// Preconditions

	ret.pre["Nop"] = { {}, true };

	Formula out_pre = Predicate("part", { Variable{"part"} }) && Predicate("at", { Variable{"part"}, Variable{"i"} });
	ret.pre["Out"] = { {Variable{"part"}, Variable{"i"}}, out_pre};

	Formula in_pre = Predicate("part", { Variable{"part"} }) &&
		!Quantifier("p", Predicate("at", { Variable{"p"}, Variable{"i"} }), QuantifierKind::Existential);
	ret.pre["In"] = { {Variable{"part"}, Variable{"i"}}, in_pre };

	// Successors

	ret.SetInitial(s0);
	return ret;
}