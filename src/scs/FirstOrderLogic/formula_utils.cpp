#include "scs/FirstOrderLogic/formula_utils.h"

#include <algorithm>

namespace scs {

	namespace {

		void MergeFormulaVariables(VariableSet& target, const VariableSet& source) {
			target.insert(source.begin(), source.end());
		}

		Term RenameTerm(const Term& term, const VariableRenaming& renaming) {
			if (const auto* variable = std::get_if<Variable>(&term)) {
				if (const auto found = renaming.find(*variable); found != renaming.end()) {
					return found->second;
				}
			}
			return term;
		}

		Variable RenameVariable(const Variable& variable, const VariableRenaming& renaming) {
			if (const auto found = renaming.find(variable); found != renaming.end()) {
				return found->second;
			}
			return variable;
		}

	}

	VariableSet FreeVariables(const Action& action) {
		VariableSet variables;
		for (const auto& term : action.terms) {
			if (const auto* variable = std::get_if<Variable>(&term)) {
				variables.emplace(*variable);
			}
		}
		return variables;
	}

	VariableSet FreeVariables(const CompoundAction& action) {
		VariableSet variables;
		for (const auto& component : action.Actions()) {
			MergeFormulaVariables(variables, FreeVariables(component));
		}
		return variables;
	}

	VariableSet FreeVariables(const Formula& formula) {
		if (const auto* variable = std::get_if<Variable>(&formula)) {
			return {*variable};
		}
		if (const auto* predicate = std::get_if<Predicate>(&formula)) {
			VariableSet variables;
			for (const auto& term : predicate->terms()) {
				if (const auto* variable = std::get_if<Variable>(&term)) {
					variables.emplace(*variable);
				}
			}
			return variables;
		}
		if (const auto* action = std::get_if<Action>(&formula)) {
			return FreeVariables(*action);
		}
		if (const auto* coop = std::get_if<CoopPredicate>(&formula)) {
			return {coop->i, coop->j};
		}
		if (const auto* unary = std::get_if<Box<UnaryConnective>>(&formula)) {
			return FreeVariables((*unary)->child());
		}
		if (const auto* binary = std::get_if<Box<BinaryConnective>>(&formula)) {
			auto variables = FreeVariables((*binary)->lhs());
			MergeFormulaVariables(variables, FreeVariables((*binary)->rhs()));
			return variables;
		}
		if (const auto* quantifier = std::get_if<Box<Quantifier>>(&formula)) {
			auto variables = FreeVariables((*quantifier)->child());
			variables.erase((*quantifier)->variable());
			return variables;
		}
		return {};
	}

	Action RenameVariables(const Action& action, const VariableRenaming& renaming) {
		Action renamed = action;
		for (auto& term : renamed.terms) {
			term = RenameTerm(term, renaming);
		}
		return renamed;
	}

	CompoundAction RenameVariables(const CompoundAction& action, const VariableRenaming& renaming) {
		CompoundAction renamed;
		for (const auto& component : action.Actions()) {
			renamed.AppendAction(RenameVariables(component, renaming));
		}
		return renamed;
	}

	Formula RenameFreeVariables(const Formula& formula, const VariableRenaming& renaming) {
		if (const auto* variable = std::get_if<Variable>(&formula)) {
			return RenameVariable(*variable, renaming);
		}
		if (const auto* predicate = std::get_if<Predicate>(&formula)) {
			auto renamed = *predicate;
			for (auto& term : renamed.terms_) {
				term = RenameTerm(term, renaming);
			}
			return renamed;
		}
		if (const auto* action = std::get_if<Action>(&formula)) {
			return RenameVariables(*action, renaming);
		}
		if (const auto* coop = std::get_if<CoopPredicate>(&formula)) {
			return CoopPredicate{RenameVariable(coop->i, renaming), RenameVariable(coop->j, renaming)};
		}
		if (const auto* unary = std::get_if<Box<UnaryConnective>>(&formula)) {
			return UnaryConnective{RenameFreeVariables((*unary)->child(), renaming), (*unary)->kind()};
		}
		if (const auto* binary = std::get_if<Box<BinaryConnective>>(&formula)) {
			return BinaryConnective{
				RenameFreeVariables((*binary)->lhs(), renaming),
				RenameFreeVariables((*binary)->rhs(), renaming),
				(*binary)->kind()};
		}
		if (const auto* quantifier = std::get_if<Box<Quantifier>>(&formula)) {
			auto child_renaming = renaming;
			child_renaming.erase((*quantifier)->variable());
			return Quantifier{(*quantifier)->variable(),
				RenameFreeVariables((*quantifier)->child(), child_renaming), (*quantifier)->kind()};
		}
		return formula;
	}

	Formula Conjoin(const Formula& lhs, const Formula& rhs) {
		if (const auto* value = std::get_if<bool>(&lhs)) {
			return *value ? rhs : Formula{false};
		}
		if (const auto* value = std::get_if<bool>(&rhs)) {
			return *value ? lhs : Formula{false};
		}
		if (lhs == rhs) {
			return lhs;
		}
		return BinaryConnective{lhs, rhs, BinaryKind::Conjunction};
	}

	Formula Disjoin(const Formula& lhs, const Formula& rhs) {
		if (const auto* value = std::get_if<bool>(&lhs)) {
			return *value ? Formula{true} : rhs;
		}
		if (const auto* value = std::get_if<bool>(&rhs)) {
			return *value ? Formula{true} : lhs;
		}
		if (lhs == rhs) {
			return lhs;
		}
		return BinaryConnective{lhs, rhs, BinaryKind::Disjunction};
	}

	Formula Negate(const Formula& formula) {
		if (const auto* value = std::get_if<bool>(&formula)) {
			return !*value;
		}
		if (const auto* unary = std::get_if<Box<UnaryConnective>>(&formula);
			unary != nullptr && (*unary)->kind() == UnaryKind::Negation) {
			return (*unary)->child();
		}
		return UnaryConnective{formula, UnaryKind::Negation};
	}

	Formula ExistentiallyQuantify(const std::vector<Variable>& variables, const Formula& formula) {
		if (std::holds_alternative<bool>(formula)) {
			return formula;
		}
		Formula result = formula;
		for (auto variable = variables.rbegin(); variable != variables.rend(); ++variable) {
			result = Quantifier{*variable, result, QuantifierKind::Existential};
		}
		return result;
	}

}
