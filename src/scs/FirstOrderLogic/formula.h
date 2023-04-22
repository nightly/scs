#pragma once

#include <variant>
#include <memory>

#include "scs/FirstOrderLogic/forward.h"
#include "scs/Memory/box.h"

#include "scs/SituationCalculus/relational_fluent.h"
#include "scs/SituationCalculus/action.h"
#include "scs/SituationCalculus/situation.h"
#include "scs/FirstOrderLogic/object.h"
#include "scs/FirstOrderLogic/variable.h"
#include "scs/FirstOrderLogic/predicate.h"

namespace scs {

	using Formula = std::variant<
		bool,
		Object,
		Variable,
		Predicate,
		Action,
		Situation,
		Box<UnaryConnective>,
		Box<BinaryConnective>,
		Box<Quantifier>
		>;

}

#include "scs/FirstOrderLogic/unary.h"
#include "scs/FirstOrderLogic/binary.h"
#include "scs/FirstOrderLogic/quantifier.h"