#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/SituationCalculus/bat.h"

#include <cstddef>

namespace scs {
	using ResourceIndex = size_t;

	struct Resource {
	public:
		ResourceIndex index = 0;
		std::shared_ptr<IProgram> program;
		BasicActionTheory bat;
	public:
		Resource() = default;
		Resource(ResourceIndex index, std::shared_ptr<IProgram> program, BasicActionTheory bat = {})
			: index(index), program(std::move(program)), bat(std::move(bat)) {}
	};

}
