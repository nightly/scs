#pragma once

#include "scs/SituationCalculus/situation_calculus.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/ConGolog/Program/programs.h"
#include "scs/ConGolog/resource.h"

#include "Hinge/common.h"

/*
#BAT

#Program
loop:
	Nop | In(part, 4); Paint(part, colour, 4); Out(part, 4)

*/

namespace scs::examples {

	inline Resource HingeGroundedResource4() {
		Resource ret;
		Situation s0;
		HingeCommon com;

		// Program ----
		scs::Action Nop{ "Nop"};
		scs::Action In{ "In", { Variable{"part"}, Object{"4"} }};
		scs::Action Out{ "Out", { Variable{"part"}, Object{"4"} }};
		scs::Action Paint{ "Paint", { Variable{"part"}, Variable{"colour"}, Object{"4"} }};
		// Grounded
		scs::ActionProgram InG1{ Action{"In", {Object{"brass"}, Object{"4"}}}};
		scs::ActionProgram InG2{ Action{"In", {Object{"tube"}, Object{"4"}}}};
		Branch InB(InG1, InG2);
		scs::ActionProgram OutG1{ Action{"Out", {Object{"brass"}, Object{"4"}}}};
		scs::ActionProgram OutG2{ Action{"Out", {Object{"tube"}, Object{"4"}}}};
		Branch OutB(OutG1, OutG2);
		scs::ActionProgram PaintG1{ Action{"Paint", {Object{"brass"}, Object{"metallic_red"}, Object{"4"}}}};
		scs::ActionProgram PaintG2{ Action{"Paint", {Object{"tube"}, Object{"metallic_red"}, Object{"4"}}}};
		scs::ActionProgram PaintG3{ Action{"Paint", {Object{"brass"}, Object{"metallic_blue"}, Object{"4"}}}};
		scs::ActionProgram PaintG4{ Action{"Paint", {Object{"tube"}, Object{"metallic_blue"}, Object{"4"}}}};
		Branch PaintB(Branch(PaintG1, PaintG2), Branch(PaintG3, PaintG4));
		// ----

		ret.bat.types["Paint"] = ActionType::Manufacturing;

		Sequence s1(InB, PaintB);
		Sequence s2(s1, OutB);
		Branch nd1(s2, ActionProgram{ Nop });
		auto prog = std::make_shared<Loop>(nd1);

		// Objects and initial valuations
		ret.bat.objects.emplace("4"); // Constant 4
		ret.bat.objects.emplace("metallic_red");
		ret.bat.objects.emplace("metallic_blue");

		// Preconditions
		Formula pre_paint = Predicate{ "at", {Variable{"part"}, Variable{"i"}} } && Predicate{"pigment", {Variable{"colour"}}};
		ret.bat.pre["Paint"] = { {scs::Variable{"part"}, scs::Variable{"colour"}, scs::Variable{"i"}}, pre_paint };

		// Successors

		ret.program = prog;
		ret.bat.SetInitial(s0);
		return ret;
	}

	inline CharacteristicGraph HingeGroundedResource4Cg() {
		// Simplified Cg (manually for now)
		CharacteristicGraph cg;
		cg.lts.AddTransition(0, { Action{"Nop"}, true }, 0);

		cg.lts.AddTransition(0, { Action{"In", {Object{"brass"}, Object{"4"}}}, true}, 1);
		cg.lts.AddTransition(0, { Action{"In", {Object{"tube"}, Object{"4"}}}, true}, 1);

		cg.lts.AddTransition(1, { Action{ "Paint", {Object{"brass"}, Object{"metallic_red"}, Object{"4"}} }, true }, 2);
		cg.lts.AddTransition(1, { Action{ "Paint", {Object{"tube"}, Object{"metallic_red"}, Object{"4"}} }, true }, 2);
		cg.lts.AddTransition(1, { Action{ "Paint", {Object{"brass"}, Object{"metallic_blue"}, Object{"4"}} }, true }, 2);
		cg.lts.AddTransition(1, { Action{ "Paint", {Object{"tube"}, Object{"metallic_blue"}, Object{"4"}} }, true }, 2);

		cg.lts.AddTransition(2, { Action{"Out", {Object{"brass"}, Object{"4"}}}, true}, 0);
		cg.lts.AddTransition(2, { Action{"Out", {Object{"tube"}, Object{"4"}}}, true}, 0);
		return cg;
	}

}
