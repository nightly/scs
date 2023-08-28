#pragma once

#include <iostream>
#include <format>

#include "scs/SituationCalculus/situation_calculus.h"
#include "scs/Common/strings.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/ConGolog/Program/programs.h"
#include "scs/ConGolog/resource.h"

#include "Hinge/common.h"

/*
#BAT

#Program
loop:
	if equipped(bit, 3) then
		RadialDrill(part, bit, diameter, 3); DetachBit(bit, 3)
	else
		AttachBit(3mm, 3) | AttachBit(5mm, 3)
	endIf |
	In(part, 3) | Out(part, 3) | ApplyAdhesive(part1, part2, 3) | Nop
*/

namespace scs::examples {

	inline Resource HingeGroundedResource3() {
		Resource ret;
		Situation s0;

		// Program
		scs::Action Nop{ "Nop"};
		scs::Action RadialDrill{"RadialDrill", { Variable{"part"},  Variable{"bit"}, Variable{"diameter"}, Object{"3"} }};
		scs::Action AttachBit3mm{"AttachBit", { Object{"3mm"}, Object{"3"} }}; // concrete action
		scs::Action AttachBit5mm{"AttachBit", { Object{"5mm"}, Object{"3"} }}; // concrete action
		scs::Action In{"In", { Variable{"part"}, Object{"3"} }};
		scs::Action Out{"Out", { Variable{"part"}, Object{"3"} }};
		scs::Action DetachBit{"DetachBit", { Variable{"bit"}, Object{"3"} }};
		scs::Action ApplyAdhesive{"ApplyAdhesive", { Variable{"part1"}, Variable{"part2"}, Object{"3"}}};

		ret.bat.types["RadialDrill"] = ActionType::Manufacturing;
		ret.bat.types["AttachBit"] = ActionType::Prepatory;
		ret.bat.types["DetachBit"] = ActionType::Prepatory;
		ret.bat.types["ApplyAdhesive"] = ActionType::Manufacturing;

		ActionProgram if1_true_chain{RadialDrill};
		Branch if1_false_chain{ ActionProgram{AttachBit3mm}, ActionProgram{AttachBit5mm} };
		Formula cond = Quantifier{ "b", Predicate{"equipped_bit", {scs::Variable{"b"}, Object{"3"} }},
			QuantifierKind::Existential };
		CgIf if1{cond, if1_true_chain, if1_false_chain };

		Branch nd2(ActionProgram{In}, ActionProgram{Out});
		Branch nd3(nd2, ActionProgram{ApplyAdhesive});
		Branch nd4(nd3, ActionProgram{Nop});
		Branch nd5(nd4, ActionProgram{DetachBit});
		Branch nd6(if1, nd5);

		// Objects and initial valuations
		ret.bat.objects.emplace("3"); // Constant 3
		ret.bat.objects.emplace("1.5");
		ret.bat.objects.emplace("3mm");
		ret.bat.objects.emplace("5mm");

		// Preconditions
		Formula pre_attach = Predicate("bit", { Variable{"bit"} }) && Not(Quantifier("b", Predicate("equipped_bit", {Variable{"b"}, Variable{"i"}}),
			QuantifierKind::Existential));
		ret.bat.pre["AttachBit"] = { {scs::Variable{"bit"}, scs::Variable{"i"}}, pre_attach };

		Formula pre_detach = Predicate("equipped_bit", {scs::Variable{"bit"}, scs::Variable{"i"}});
		ret.bat.pre["DetachBit"] = { {scs::Variable{"bit"}, scs::Variable{"i"}}, pre_detach};

		HingeCommon com;
		Formula pre_drill = Predicate("material", { Variable{"part"} }) && Predicate("suitable", { Variable{"bit"}, Variable{"diameter"} }) &&
			Predicate("equipped_bit", {Variable{"bit"}, Variable{"i"}}) &&
			Quantifier("f", Quantifier("c", Predicate("clamped", {Variable{"part"}, Variable{"f"},
				Variable{"c"}}), QuantifierKind::Existential), QuantifierKind::Existential) && com.within_reach;
		// add within_reach and remove clamped's i 
		ret.bat.pre["RadialDrill"] = { {Variable{"part"}, Variable{"bit"}, Variable{"diameter"}, Variable{"i"}}, pre_drill};

		Formula pre_apply_adhesive = com.within_reach_part_1 && com.within_reach_part_2 &&
			Predicate("part", { Variable{"part1"} }) && Predicate("part", { Variable{"part2"} }) && Not(Quantifier("j", Quantifier("f",
				(Predicate("clamped", {Variable{"part1"}, Variable{"f"}, Variable{"j"}}) || 
				Predicate("clamped", { Variable{"part2"}, Variable{"f"}, Variable{"j"} })), 
			QuantifierKind::Existential), QuantifierKind::Existential));
		ret.bat.pre["ApplyAdhesive"] = { {Variable{"part1"}, Variable{"part2"}, Variable{"i"}}, pre_apply_adhesive};

		// Successors
		Formula bit_pos = a_eq(Action("AttachBit", {Variable{"bit"}, Variable{"i"}}));
		Formula bit_neg = cv() && Not(a_eq(Action("DetachBit", {Variable{"bit"}, Variable{"i"}} )));
		ret.bat.successors["equipped_bit"] = { {Variable{"bit"}, Variable{"i"}}, bit_pos || bit_neg};

		auto prog = std::make_shared<Loop>(nd6);
		ret.program = prog;
		ret.bat.SetInitial(s0);
		return ret;
	}

}