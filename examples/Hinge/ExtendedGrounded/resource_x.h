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
	Load(part, 2) | Nop∗ | Out(part, 2)
*/

namespace scs::examples {

	inline Resource HingeGroundedResourceX() {
		Resource ret;
		Situation s0;

		// Program
		scs::Action Nop{ "Nop", {} };

		ActionProgram OutG1{ Action{"Out", {Object{"brass"}, Object{"5"}}} };
		ActionProgram OutG2{ Action{"Out", {Object{"tube"}, Object{"5"}}} };
		Branch OutB(OutG1, OutG2);
		ActionProgram ReamingG1(Action{ "Reaming", { Object{"brass"}, Object{"5"}} });
		ActionProgram ReamingG2(Action{ "Reaming", { Object{"tube"}, Object{"5"}} });
		Branch LoadB(ReamingG1, ReamingG2);

		// 
		ret.bat.types["Reaming"] = ActionType::Manufacturing;

		scs::Loop l1(ActionProgram{ Nop }); // Nop*
		scs::Branch nd1(LoadB, l1); // Load | Nop*
		Branch nd2(nd1, OutB);

		// Objects and initial valuations
		ret.bat.objects.emplace("5"); // Constant 5

		HingeCommon com;
		// Preconditions
		// Formula pre_reaming = Predicate{ "at", {Variable{"part"}, Variable{"i"}} };
		Formula pre_reaming = com.within_reach;
		ret.bat.pre["Reaming"] = { {scs::Variable{"part"}, scs::Variable{"i"}}, pre_reaming };

		// Successors

		//////////////
		ret.program = std::make_shared<Loop>(nd2);
		ret.bat.SetInitial(s0);
		return ret;
	}

}