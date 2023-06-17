#pragma once

#include "scs/ConGolog/Program/programs.h"
#include "scs/ConGolog/execution.h"

namespace scs {

	struct RecipeVisitor {
	public:

		bool is_complete = false;
	public:
		RecipeVisitor() {

		}

		void operator() (const Action& a) {
			if (a.name == "LastOp") {

			} else {
			
			}
		}



	};

}

// Could use either program* or represent recipe as execution trace