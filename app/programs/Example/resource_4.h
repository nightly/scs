#pragma once

#include <iostream>
#include <format>

#include "scs/SituationCalculus/situation_calculus.h"
#include "scs/Common/strings.h"
#include "scs/FirstOrderLogic/fol.h"
#include "scs/ConGolog/Program/programs.h"
#include "scs/ConGolog/resource.h"

using namespace scs;


/*
#BAT



#Program
loop:
	in(part,1)
	| if ¬equipd(driller,1) then equip(driller,1) endIf;
	nop*;
	if ¬has_bit(bit_t ype,dmtr, driller, 1) then set_bit(bit, bit_t ype,dmtr,1) endIf;
	robot_drill(part, bit,dmtr, speed, f eed, x, y, z,1)
	| if ¬equipd(rivet_gun, 1) then equip(rivet_gun, 1) endIf;
	nop*; rivet(part,rivet_type, x, y, z, 1);
	if ¬charged(compressor,1) then start_compressor(1) endIf
	| out(part,1)
	| unequip(1)
	| nop

*/

inline Resource ExampleResource4() {
	Resource ret;
	Situation s0;

	// Program
	scs::Action Nop{ "Nop", {} };
	scs::Action In{"In", { scs::Variable{"part"}, scs::Object{"1"} }};
	scs::Action Out{"Out", { scs::Variable{"part"}, scs::Object{"1"} }};
	scs::Action EquipDriller{"Equip", { scs::Object{"driller"}, scs::Object{"1"} }};
	scs::Action EquipRivet{"Equip", { scs::Object{"rivet_gun"}, scs::Object{"1"} }};
	scs::Action SetBit{ "SetBit", { scs::Variable{"bit"}, scs::Variable{"bit_type"}, scs::Variable{"dmtr"}, scs::Object{"1"} }};
	scs::Action RobotDrill{ "RobotDrill", { scs::Variable{"part"}, scs::Variable{"bit"}, scs::Variable{"dmtr"}, scs::Variable{"speed"}, scs::Variable{"feed"},
		scs::Variable{"x"}, scs::Variable{"y"}, scs::Variable{"z"}, scs::Object{"1"} }};
	scs::Action Unequip{ "Unequip", { scs::Object{"1"} } };
	scs::Action StartCompressor{ "StartCompressor", { scs::Object{"1"} } };


	// Objects and initial valuations
	s0.objects.emplace("1"); // Constant 1


	// Preconditions

	// Successors




	return ret;
}
