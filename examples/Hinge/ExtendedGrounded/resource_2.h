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

	inline Resource HingeGroundedResource2() {
		Resource ret;
		Situation s0;

		// Program
		scs::Action Load{ "Load", { Variable{"part"}, scs::Object{"2"} }};
		scs::Action Nop{ "Nop", {} };
		scs::Action Out{ "Out", { Variable{"part"}, Object{"2"} } };

		ActionProgram OutG1{ Action{"Out", {Object{"brass"}, Object{"2"}}} };
		ActionProgram OutG2{ Action{"Out", {Object{"tube"}, Object{"2"}}} };
		Branch OutB(OutG1, OutG2);
		ActionProgram LoadG1(Action{"Load", { Object{"brass"}, Object{"2"}}});
		ActionProgram LoadG2(Action{"Load", { Object{"tube"}, Object{"2"}}});
		Branch LoadB(LoadG1, LoadG2);

		// 
		ret.bat.types["Load"] = ActionType::Manufacturing;
		

		scs::Loop l1(ActionProgram{ Nop }); // Nop*
		scs::Branch nd1(LoadB, l1); // Load | Nop*
		Branch nd2(nd1, OutB);

		// Objects and initial valuations
		ret.bat.objects.emplace("2"); // Constant 2

		HingeCommon com;
		// Preconditions
		Formula pre_load = Predicate("part", { scs::Variable{"part"} }) && Predicate("on_site", { Variable{"part"} });
		ret.bat.pre["Load"] = { std::vector<Term>{Variable{"part"}, Variable{"i"}}, pre_load };

		// Successors
		Formula on_site_neg = cv() && Not(Quantifier("i", a_eq(scs::Action{"Load", { Variable{"part"}, Variable{"i"} }}),
			QuantifierKind::Existential)) && Not(Quantifier("s", Quantifier("i", a_eq(scs::Action{"Store", {Variable{"part"}, Variable{"s"}, Variable{"i"}}}),
				QuantifierKind::Existential), QuantifierKind::Existential));
		ret.bat.successors["on_site"] = { {Variable{"part"}}, on_site_neg };
	

		//////////////
		ret.program = std::make_shared<Loop>(nd2);
		ret.bat.SetInitial(s0);
		return ret;
	}

}