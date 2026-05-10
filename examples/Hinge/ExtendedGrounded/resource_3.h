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

		ActionProgram AdhesiveG1(Action{"ApplyAdhesive", {Object{"brass"}, Object{"tube"}, Object{"3"}}});
		ActionProgram AdhesiveG2(Action{"ApplyAdhesive", {Object{"tube"}, Object{"brass"}, Object{"3"}}});
		Branch AdhesiveB(AdhesiveG1, AdhesiveG2);
		ActionProgram InG1{ Action{"In", {Object{"brass"}, Object{"3"}}} };
		ActionProgram InG2{ Action{"In", {Object{"tube"}, Object{"3"}}} };
		Branch InB(InG1, InG2);
		ActionProgram OutG1{ Action{"Out", {Object{"brass"}, Object{"3"}}} };
		ActionProgram OutG2{ Action{"Out", {Object{"tube"}, Object{"3"}}} };
		Branch OutB(OutG1, OutG2);
		ActionProgram RadialDrillG1{ Action{"RadialDrill", { Object{"brass"}, Object{"5mm"}, Object{"1.5"}, Object{"3"}}}};
		ActionProgram RadialDrillG2{ Action{"RadialDrill", { Object{"brass"}, Object{"5mm"}, Object{"5"}, Object{"3"}}}};
		Branch RadialDrillB(RadialDrillG1, RadialDrillG2);
		ActionProgram DetachBitG1(Action{"DetachBit", {Object{"3mm"}, Object{"3"}}});
		ActionProgram DetachBitG2(Action{"DetachBit", {Object{"5mm"}, Object{"3"}}});
		Branch DetachBitB(DetachBitG1, DetachBitG2);

		Branch if1_false_chain{ ActionProgram{AttachBit3mm}, ActionProgram{AttachBit5mm} };
		Formula cond = ParseScFormula("exists b. equipped_bit(b, obj(3))");
		CgIf if1{cond, RadialDrillB, if1_false_chain };

		Branch nd2(InB, OutB);
		Branch nd3(nd2, AdhesiveB);
		Branch nd4(nd3, ActionProgram{Nop});
		Branch nd5(nd4, DetachBitB);
		Branch nd6(if1, nd5);

		ret.bat = ParseBasicActionTheory(R"(
objects 3, 1.5, 3mm, 5mm
type RadialDrill = manufacturing
type AttachBit = preparatory
type DetachBit = preparatory
type ApplyAdhesive = manufacturing

poss AttachBit(bit, i) = bit(bit) and not (exists b. equipped_bit(b, i))
poss DetachBit(bit, i) = equipped_bit(bit, i)
poss RadialDrill(part, bit, diameter, i) = material(part) and suitable(bit, diameter) and equipped_bit(bit, i) and (exists f, c. clamped(part, f, c)) and (exists j. j != i and at(part, j) and coop(i, j))
poss ApplyAdhesive(part1, part2, i) = (exists j. j != i and at(part1, j) and coop(i, j)) and (exists j. j != i and at(part2, j) and coop(i, j)) and part(part1) and part(part2) and not (exists j, f. clamped(part1, f, j) or clamped(part2, f, j))

ssa equipped_bit(bit, i) = a = AttachBit(bit, i) or (cv and not (a = DetachBit(bit, i)))
)");

		auto prog = std::make_shared<Loop>(nd6);
		ret.program = prog;
		return ret;
	}

}
