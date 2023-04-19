#include "scs/ConGolog/Program/configuration.h"

#include "scs/SituationCalculus/situation.h"
#include "scs/ConGolog/Program/interface_program.h"

namespace scs {

	Configuration::Configuration(const IProgram* prog, const Situation& s) 
		: prog(prog), sit(s) {}

	Configuration::Configuration(const IProgram* prog, Situation&& s)
		: prog(prog), sit(std::move(s)) {}

}