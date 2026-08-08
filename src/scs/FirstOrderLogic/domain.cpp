#include "scs/FirstOrderLogic/domain.h"

#include "scs/SituationCalculus/bat.h"

namespace scs {

	const Domain::ObjectSet& Domain::Objects() const {
		return object_universe == nullptr ? bat->objects : *object_universe;
	}

}
