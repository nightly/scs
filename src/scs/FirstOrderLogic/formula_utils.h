#pragma once

#include <unordered_map>
#include <unordered_set>

#include "scs/FirstOrderLogic/fol.h"
#include "scs/SituationCalculus/compound_action.h"

namespace scs {

	using VariableSet = std::unordered_set<Variable>;
	using VariableRenaming = std::unordered_map<Variable, Variable>;

	VariableSet FreeVariables(const Formula& formula);
	size_t QuantifiedVariableCount(const Formula& formula);
	VariableSet FreeVariables(const Action& action);
	VariableSet FreeVariables(const CompoundAction& action);

	Formula RenameFreeVariables(const Formula& formula, const VariableRenaming& renaming);
	Action RenameVariables(const Action& action, const VariableRenaming& renaming);
	CompoundAction RenameVariables(const CompoundAction& action, const VariableRenaming& renaming);

	Formula Conjoin(const Formula& lhs, const Formula& rhs);
	Formula Disjoin(const Formula& lhs, const Formula& rhs);
	Formula Negate(const Formula& formula);
	Formula ExistentiallyQuantify(const std::vector<Variable>& variables, const Formula& formula);
	Formula IsIdentifier(const Term& term);

}
