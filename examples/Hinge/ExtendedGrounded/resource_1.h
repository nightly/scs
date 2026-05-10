#pragma once

#include <iostream>
#include <format>

#include "scs/SituationCalculus/situation_calculus.h"
#include "scs/Common/strings.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/ConGolog/Program/programs.h"
#include "scs/ConGolog/resource.h"

/*
#BAT

#Program
loop:
	Nop* | In(part, 1); (Nop | (Clamp(part, force, 1); Release(part, 1))*; (Out(part, 1) | Store(part, code, 1))
*/

namespace scs::examples {

	inline Resource HingeGroundedResource1() {
		Resource ret;
		Situation s0;

		// Program ----
		scs::Action Nop{ "Nop", {} };
		scs::Action In{ "In", { Variable{"part"}, Object{"1"} }};
		scs::Action Out{ "Out", { Variable{"part"}, Object{"1"} }};
		scs::Action Clamp{ "Clamp", { Variable{"part"}, Variable{"force"}, Object{"1"} }};
		scs::Action Release{ "Release", { Variable{"part"}, Object{"1"} }};
		scs::Action Store{ "Store", { Variable{"part"}, Variable{"code"}, Object{"1"} } };
		// Grounded
		ActionProgram InG1{ Action{"In", {Object{"brass"}, Object{"1"}}} };
		ActionProgram InG2{ Action{"In", {Object{"tube"}, Object{"1"}}} };
		Branch InB(InG1, InG2);
		ActionProgram OutG1{ Action{"Out", {Object{"brass"}, Object{"1"}}} };
		ActionProgram OutG2{ Action{"Out", {Object{"tube"}, Object{"1"}}} };
		Branch OutB(OutG1, OutG2);
		ActionProgram StoreG1(Action{ "Store", { Object{"brass"}, Object{"ok"}, Object{"1"}}});
		ActionProgram StoreG2(Action{ "Store", { Object{"tube"}, Object{"ok"}, Object{"1"}}});
		Branch StoreB(StoreG1, StoreG2);
		ActionProgram ClampG1(Action{ "Clamp", { Object{"brass"}, Object{"5"}, Object{"1"}}});
		ActionProgram ClampG2(Action{ "Clamp", { Object{"tube"}, Object{"5"}, Object{"1"}}});
		Branch ClampB(ClampG1, ClampG2);
		ActionProgram ReleaseG1(Action{"Release", {Object{"tube"}, Object{"1"}}});
		ActionProgram ReleaseG2(Action{"Release", {Object{"brass"}, Object{"1"}}});
		Branch ReleaseB(ReleaseG1, ReleaseG2);
		// ----

		Branch nd1(ActionProgram{ Nop }, Sequence(ClampB, ReleaseB));
		Loop l1(nd1);
		Sequence s1(InB, l1);
		Sequence s2(s1, Branch(OutB, StoreB));

		Branch nd2(ActionProgram{ Nop }, s2);

		ret.bat = ParseBasicActionTheory(R"(
objects 1, ok
type Clamp = preparatory
type Release = preparatory
type Store = manufacturing

init safe_force(brass, 5) = true
init safe_force(tube, 5) = true
init safe_force(tube, 0.5) = true

poss Clamp(part, force, i) = part(part) and at(part, i) and safe_force(part, force) and not (exists f. clamped(part, f, i))
poss Release(part, i) = part(part) and at(part, i) and exists f. clamped(part, f, i)
poss Store(part, code, i) = at(part, i) and status(code)

ssa clamped(part, force, i) = a = Clamp(part, force, i) or (cv and not (a = Release(part, i)))
ssa part(part) = cv and not (exists i, code. a = Store(part, code, i)) and not (exists other, i. a = ApplyAdhesive(other, part, i))
)");

		ret.program = std::make_shared<Loop>(nd2);
		return ret;
	}

	inline CharacteristicGraph HingeGroundedResource1Cg() {
		// Simplified Cg (manually for now)
		CharacteristicGraph cg;
		cg.lts.AddTransition(0,{Action{"Nop"}, true }, 0);


		cg.lts.AddTransition(2,{ Action{ "Clamp", { Object{"brass"}, Object{"5"}, Object{"1"}}}, true }, 4);
		cg.lts.AddTransition(2,{ Action{ "Clamp", { Object{"tube"}, Object{"5"}, Object{"1"}}}, true }, 4);

		cg.lts.AddTransition(4, { Action{"Release", {Object{"brass"}, Object{"1"}}}, true}, 2);
		cg.lts.AddTransition(4, { Action{"Release", {Object{"tube"}, Object{"1"}}}, true}, 2);

		cg.lts.AddTransition(0, { Action{"In", {Object{"brass"}, Object{"1"}}}, true}, 2);
		cg.lts.AddTransition(0, { Action{"In", {Object{"tube"}, Object{"1"}}}, true}, 2);
		
		cg.lts.AddTransition(2, { Action{"Out", {Object{"brass"}, Object{"1"}}}, true }, 0);
		cg.lts.AddTransition(2, { Action{"Out", {Object{"tube"}, Object{"1"}}}, true }, 0);
		
		cg.lts.AddTransition(2, { Action{ "Store", { Object{"brass"}, Object{"ok"}, Object{"1"}}}, true}, 0);
		cg.lts.AddTransition(2, { Action{ "Store", { Object{"tube"}, Object{"ok"}, Object{"1"}}}, true}, 0);
		return cg;
	}

}
