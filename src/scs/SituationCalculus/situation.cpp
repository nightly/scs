﻿#include "scs/SituationCalculus/situation.h"

#include <vector>
#include <string>

#include "scs/Common/log.h"
#include "scs/SituationCalculus/action.h"

#include "scs/SituationCalculus/poss.h"
#include "scs/SituationCalculus/successor.h"
#include "scs/FirstOrderLogic/evaluator.h"
#include "scs/SituationCalculus/bat.h"

namespace scs {

	void Situation::AddFluent(const std::string& name, const RelationalFluent& fluent) {
		relational_fluents_[name] = fluent;
	}

	void Situation::AddFluent(const std::string& name, RelationalFluent&& fluent) {
		relational_fluents_[name] = std::move(fluent);
	}

	bool Situation::ObjectInDomain(const Object& o, const BasicActionTheory& bat) const {
		return bat.objects.contains(o);
	}

	size_t Situation::Length() const {
		return history.size();
	}

	bool Situation::Possible(const Action& a, const BasicActionTheory& bat) const {
		FirstOrderAssignment assignment;
		assert(bat.pre.contains(a.name) && "Missing precondition for action type");
		assert((bat.pre.at(a.name).Terms().size() == a.terms.size()) && "Number of terms different in Poss vs action");
		const auto& poss = bat.pre.at(a.name);

		for (size_t i = 0; i < a.terms.size(); ++i) {
			if (const scs::Variable* var_ptr = std::get_if<Variable>(&poss.Terms().at(i))) {
				const auto& obj = std::get<Object>(a.terms[i]); // Performing an action, must have complete object literals
				assert(ObjectInDomain(obj, bat) && "Object not within domain");
				assignment.Set(*var_ptr, obj);
			}
		}
		scs::Evaluator eval{ {*this, bat, bat.CoopMx(), bat.RoutesMx()}, assignment };
		return std::visit(eval, poss.Form());
	}

	bool Situation::Possible(const CompoundAction& ca, const BasicActionTheory& bat) const {
		// Poss(a_1 \cup a_2, s), we check all preconditions instantly, aside from some special mappings
		for (const auto& entry : bat.poss_mappings.mappings) {
			if (entry.contain_check(ca)) {
				return entry.action_check(*this, ca, bat);
			}
		}

		for (const auto& act : ca.Actions()) {
			bool local = this->Possible(act, bat);
			if (!local) {
				return false;
			}
		}
		return true;
	}

	const Situation::u_map<std::string, RelationalFluent>& Situation::Fluents() const {
		return relational_fluents_;
	}

	/**
	 * @brief: Do performs the action a in the current situation s
	 * Rechecking preconditions is not done (it is assumed to be done elsewhere along the chain) so we assert Poss instead.
	 */
	Situation Situation::Do(const Action& a, const BasicActionTheory& bat) const {
		Situation next = *this;
		next.history.emplace_back(a);

		for (const auto& [fluent_name, successor] : bat.successors) {
			if (successor.Involves(a)) {
				auto& fluent = next.relational_fluents_[fluent_name]; // inplace from next situation
				for (auto& [fluent_objects, fluent_value] : fluent.valuations()) {
					SCS_TRACE("{} = {}", fluent_objects, fluent_value);
					assert(successor.Terms().size() == fluent_objects.size() && "Number of terms in successor is different to number of terms in fluent valuation");

					FirstOrderAssignment assignment;
					for (size_t i = 0; i < successor.Terms().size(); ++i) {
						if (auto* var_ptr = std::get_if<Variable>(&successor.Terms().at(i))) {
							const auto& obj = fluent_objects.at(i);
							assignment.Set(*var_ptr, obj);
						}
					}

					fluent_value = successor.Evaluate(fluent_value, *this, bat, a, assignment);
				}
			}
		}

		return next;
	}

	Situation Situation::Do(const CompoundAction& ca, const BasicActionTheory& bat) const {
		Situation next = *this;
		next.history.emplace_back(ca);

		for (const auto& [fluent_name, successor] : bat.successors) {
			if (successor.Involves(ca)) {
				auto& fluent = next.relational_fluents_[fluent_name]; // inplace from next situation
				for (auto& [fluent_objects, fluent_value] : fluent.valuations()) {
					SCS_TRACE("{} = {}", fluent_objects, fluent_value);
					assert(successor.Terms().size() == fluent_objects.size() && "Number of terms in successor is different to number of terms in fluent valuation");

					FirstOrderAssignment assignment;
					for (size_t i = 0; i < successor.Terms().size(); ++i) {
						if (auto* var_ptr = std::get_if<Variable>(&successor.Terms().at(i))) {
							const auto& obj = fluent_objects.at(i);
							assignment.Set(*var_ptr, obj);
						}
					}

					fluent_value = successor.Evaluate(fluent_value, *this, bat, ca, assignment);
				}
			}
		}

		return next;
	}

	void Situation::PrintHistory(std::ostream& os) const {
		os << "do[";

		for (size_t i = 0; i < history.size(); ++i) {
			const auto& act = history[i];
			os << act;
			if (i != history.size() - 1) {
				os << ", ";
			}
		}

		os << "]";
	}

	void Situation::PrintFluents(std::ostream& os, bool with_history, size_t indent) const {
		std::string indent_space(indent, ' ');
		if (with_history) {
			os << indent_space << "Fluents for situation: ";
			PrintHistory(os);
			os << "\n";
		} else {
			os << indent_space << "Fluents = ";
			os << "\n";
		}
		indent_space = std::string(indent + 1, ' ');
		for (const auto& f : this->relational_fluents_) {
			os << indent_space << f.first << "{";
			os << f.second;
			os << "}";
			os << "\n";
		}
	}


	/*
	* Operator overloads
	*/

	bool Situation::operator==(const Situation& other) const {
		return history == other.history; // Histories alone suffice for comparison
	}

	bool Situation::operator!=(const Situation& other) const {
		return history != other.history;
	}

	std::ostream& operator<< (std::ostream& os, const Situation& sit) {
		sit.PrintHistory(os);
		os << ":\n";
		sit.PrintFluents(os, false, 1);
		return os;
	}

	std::ostream& operator<< (std::ostream& os, const std::variant<Action, CompoundAction>& act) {
		if (auto* a = std::get_if<Action>(&act)) {
			os << *a;
		} else if (auto* ca = std::get_if<CompoundAction>(&act)) {
			os << *ca;
		}
		return os;
	}

}
