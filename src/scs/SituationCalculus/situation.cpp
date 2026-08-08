#include "scs/SituationCalculus/situation.h"

#include <vector>
#include <string>
#include <functional>
#include <stdexcept>

#include "scs/Common/log.h"
#include "scs/SituationCalculus/action.h"

#include "scs/SituationCalculus/poss.h"
#include "scs/SituationCalculus/successor.h"
#include "scs/FirstOrderLogic/evaluator.h"
#include "scs/SituationCalculus/bat.h"
#include "scs/SituationCalculus/object_universe.h"

namespace {

	void ForEachTuple(const std::vector<scs::Object>& objects, size_t arity,
		const std::function<void(const std::vector<scs::Object>&)>& callback) {
		std::vector<scs::Object> tuple;
		tuple.reserve(arity);
		const auto generate = [&](this const auto& self) -> void {
			if (tuple.size() == arity) {
				callback(tuple);
				return;
			}
			for (const auto& object : objects) {
				tuple.emplace_back(object);
				self();
				tuple.pop_back();
			}
		};
		generate();
	}

	template <typename ActionLike>
	scs::Situation Progress(const scs::Situation& current, const ActionLike& action,
		const scs::BasicActionTheory& bat, bool markovian_situations) {
		scs::Situation next;
		next.relational_fluents_ = current.relational_fluents_;
		if (!markovian_situations) {
			next.history = current.history;
			next.history.emplace_back(action);
		}

		const auto object_set = scs::RelevantObjects(current, bat, action);
		const std::vector<scs::Object> objects(object_set.begin(), object_set.end());
		for (const auto& [fluent_name, successor] : bat.successors) {
			if (!successor.Involves(action)) {
				continue;
			}
			const auto old_it = current.relational_fluents_.find(fluent_name);
			if (old_it == current.relational_fluents_.end()) {
				throw std::invalid_argument("Missing fluent '" + fluent_name + "' required by its successor-state axiom");
			}
			const auto& old_fluent = old_it->second;
			if (old_fluent.Arity() != successor.Terms().size()) {
				throw std::invalid_argument("Fluent '" + fluent_name + "' has an arity different from its successor-state axiom");
			}

			scs::RelationalFluent rebuilt(old_fluent.Arity());
			ForEachTuple(objects, old_fluent.Arity(), [&](const std::vector<scs::Object>& tuple) {
				scs::FirstOrderAssignment assignment;
				for (size_t i = 0; i < successor.Terms().size(); ++i) {
					if (const auto* variable = std::get_if<scs::Variable>(&successor.Terms()[i])) {
						assignment.Set(*variable, tuple[i]);
					}
				}
				const bool value = successor.Evaluate(old_fluent.Valuation(tuple), current, bat,
					action, assignment, &object_set);
				if (value) {
					rebuilt.AddValuation(tuple, true);
				}
			});
			next.relational_fluents_[fluent_name] = std::move(rebuilt);
		}
		return next;
	}

}

namespace scs {

	void Situation::AddFluent(const std::string& name, const RelationalFluent& fluent) {
		relational_fluents_[name] = fluent;
	}

	void Situation::AddFluent(const std::string& name, RelationalFluent&& fluent) {
		relational_fluents_[name] = std::move(fluent);
	}

	bool Situation::ObjectInDomain(const Object& o, const BasicActionTheory& bat) const {
		return RelevantObjects(*this, bat).contains(o);
	}

	size_t Situation::Length() const {
		return history.size();
	}

	bool Situation::Possible(const Action& a, const BasicActionTheory& bat) const {
		const auto objects = RelevantObjects(*this, bat, a);
		FirstOrderAssignment assignment;
		assert(bat.pre.contains(a.name) && "Missing precondition for action type");
		assert((bat.pre.at(a.name).Terms().size() == a.terms.size()) && "Number of terms different in Poss vs action");
		const auto& poss = bat.pre.at(a.name);

		for (size_t i = 0; i < a.terms.size(); ++i) {
			if (const scs::Variable* var_ptr = std::get_if<Variable>(&poss.Terms().at(i))) {
				const auto& obj = std::get<Object>(a.terms[i]); // Performing an action, must have complete object literals
				assignment.Set(*var_ptr, obj);
			}
		}
		scs::Evaluator eval{ {*this, bat, bat.CoopMx(), bat.RoutesMx(), objects}, assignment };
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
	Situation Situation::Do(const Action& a, const BasicActionTheory& bat, bool markovian_situations) const {
		return Progress(*this, a, bat, markovian_situations);
	}

	Situation Situation::Do(const CompoundAction& ca, const BasicActionTheory& bat, bool markovian_situations) const {
		return Progress(*this, ca, bat, markovian_situations);
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
