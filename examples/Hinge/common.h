#pragma once

#include <iostream>
#include <format>

#include "scs/SituationCalculus/situation_calculus.h"
#include "scs/Common/strings.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/ConGolog/Program/programs.h"
#include "scs/ConGolog/resource.h"

namespace scs::examples {

	struct HingeCommon {
	public:
		Formula Nop_form = true;

		Formula wr1 = BinaryConnective(Variable("j"), Variable("i"), BinaryKind::NotEqual) && Predicate{"at", {scs::Variable{"part"}, scs::Variable{"j"}}} &&
			CoopPredicate(scs::Variable{"i"}, scs::Variable{"j"});
		Formula within_reach = Quantifier("j", wr1, QuantifierKind::Existential);

		// Util (naming differences)
		Formula wr_part1 = BinaryConnective(Variable("j"), Variable("i"), BinaryKind::NotEqual) && 
			Predicate { "at", { scs::Variable{"part1"}, scs::Variable{"j"} } }&&
			CoopPredicate(scs::Variable{"i"}, scs::Variable{"j"});
		Formula within_reach_part_1 = Quantifier("j", wr_part1, QuantifierKind::Existential);

		Formula wr_part2 = BinaryConnective(Variable("j"), Variable("i"), BinaryKind::NotEqual) && 
			Predicate { "at", { scs::Variable{"part2"}, scs::Variable{"j"} } }&&
			CoopPredicate(scs::Variable{"i"}, scs::Variable{"j"});
		Formula within_reach_part_2 = Quantifier("j", wr_part2, QuantifierKind::Existential);
	};

	inline BasicActionTheory HingeCommonBAT() {
		BasicActionTheory bat;
		Situation s0;

		bat.types["Nop"] = ActionType::Nop;
		bat.types["In"] = ActionType::Transfer;
		bat.types["Out"] = ActionType::Transfer;

		// Objects & initial valuations
		bat.objects.emplace("brass");
		bat.objects.emplace("tube");
		bat.objects.emplace("5");

		s0.relational_fluents_["on_site"].AddValuation({ Object{"brass"} }, true);
		s0.relational_fluents_["on_site"].AddValuation({ Object{"tube"} }, true);

		s0.relational_fluents_["part"].AddValuation({ Object{"brass"} }, true);
		s0.relational_fluents_["part"].AddValuation({ Object{"tube"} }, true);

		s0.relational_fluents_["material"].AddValuation({ Object{"brass"} }, true);

		// bit_type, diameter
		s0.relational_fluents_["suitable"].AddValuation({ Object{"5mm"}, Object{"1.5"}}, true);

		s0.relational_fluents_["clamped"].AddValuation({ Object{"brass"}, Object{"5"}, Object{"1"} }, false);
		s0.relational_fluents_["clamped"].AddValuation({ Object{"tube"}, Object{"5"}, Object{"1"}}, false);
		s0.relational_fluents_["clamped"].AddValuation({ Object{"tube"}, Object{"0.5"}, Object{"1"}}, false);

		s0.relational_fluents_["bit"].AddValuation({ Object{ "3mm" } }, true);
		s0.relational_fluents_["bit"].AddValuation({ Object{ "5mm" } }, true);
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

		s0.relational_fluents_["status"].AddValuation({ Object{"ok"} }, true);

		s0.relational_fluents_["pigment"].AddValuation({ Object{"metallic_blue"} }, true);
		s0.relational_fluents_["pigment"].AddValuation({ Object{"metallic_red"} }, true);

		// Preconditions

		bat.pre["Nop"] = { {}, true };

		Formula out_pre = Predicate("part", { Variable{"part"} }) && Predicate("at", { Variable{"part"}, Variable{"i"} });
		bat.pre["Out"] = { {Variable{"part"}, Variable{"i"}}, out_pre};

		Formula in_pre = Predicate("part", { Variable{"part"} }) &&
			Not(Quantifier("p", Predicate("at", { Variable{"p"}, Variable{"i"} }), QuantifierKind::Existential));
		bat.pre["In"] = { {Variable{"part"}, Variable{"i"}}, in_pre };

		// Successors
		Formula at_pos = a_eq(scs::Action{"In", { Variable{"part"}, Variable{"i"} }}) || a_eq(scs::Action{"Load", { Variable{"part"} ,Variable{"i"} }});
		Formula at_neg = cv() && a_neq(scs::Action{"Out", { Variable{"part"}, Variable{"i"} }});
		bat.successors["at"] = { {Variable{"part"}, Variable{"i"}}, at_pos || at_neg };

		Formula on_site_neg = cv() && Not(Quantifier("i", a_eq(Action{"Load", {Variable{"part"}, Variable{"i"}}}), 
			QuantifierKind::Existential));
		bat.successors["on_site"] = { {Variable{"part"} }, on_site_neg };

		bat.SetInitial(s0);
		return bat;
	}
}