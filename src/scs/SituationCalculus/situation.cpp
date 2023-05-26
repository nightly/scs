#include "scs/SituationCalculus/situation.h"

#include <vector>
#include <string>

#include "scs/SituationCalculus/action.h"
#include "scs/Common/log.h"

#include "scs/SituationCalculus/poss.h"
#include "scs/SituationCalculus/successor.h"
#include "scs/FirstOrderLogic/evaluator.h"
#include "scs/SituationCalculus/bat.h"

namespace scs {

	void Situation::AddFluent(const RelationalFluent& fluent) {
		relational_fluents_[fluent.name()] = fluent;
	}

	void Situation::AddFluent(RelationalFluent&& fluent) {
		if (relational_fluents_.contains(fluent.name())) {
			relational_fluents_[fluent.name()] = fluent;
		} else {
			relational_fluents_.emplace(fluent.name(), std::move(fluent));
		}
	}

	bool Situation::ObjectInDomain(const Object& o) const {
		return objects.contains(o);
	}

	size_t Situation::Length() const {
		return history.size();
	}

	bool Situation::Possible(const Action& a, const BasicActionTheory& bat) const {
		FirstOrderAssignment assignment;
		const auto& poss = bat.pre.at(a.name);

		for (size_t i = 0; i < a.parameters.size(); ++i) {
			const auto& obj = std::get<Object>(a.parameters[i]); // Performing an action, must have complete object literals
			assert(ObjectInDomain(obj) && "Object not within domain");
			if (const scs::Variable* var_ptr = std::get_if<Variable>(&poss.Terms().at(i))) {
				assignment.Set(*var_ptr, obj);
			}
		}
		scs::Evaluator eval{ *this, assignment };
		return std::visit(eval, poss.Form());
	}

	bool Situation::Possible(const CompoundAction& ca, const BasicActionTheory& bat) const {
		// Poss(a_1 \cup a_2, s), we check all preconditions instantly such that they are independent (but sequentially)
		for (const auto& act : ca.Actions()) {
			bool local = this->Possible(act, bat);
			if (!local) {
				return false;
			}
		}
		return true;
	}

	/**
	 * @brief: Do performs the action a in the current situation s
	 * Rechecking preconditions is not done (it is assumed to be done elsewhere along the chain) so we assert Poss instead.
	 */
	Situation Situation::Do(const Action& a, const BasicActionTheory& bat) const {
		Situation next = *this;
		next.history.emplace_back(a);

		for (const auto& successor : bat.successors) {
			if (successor.second.Involves(a)) {
				auto& fluent = next.relational_fluents_[successor.first];
				for (auto& valuations : fluent.valuations()) {
					// Update inplace as we're already next situation
					valuations.second = successor.second.Evaluate(valuations.second, a, *this);
				}
			}
		}
		

		return next;
	}

	Situation Situation::Do(const CompoundAction& ca, const BasicActionTheory& bat) const {
		Situation sit = *this;
		for (const auto& act : ca.Actions()) {
			sit = sit.Do(act, bat);
		}
		return sit;
	}

	void Situation::PrintObjects(std::ostream& output_stream) const {
		output_stream << "Objects in situation: ";
		ObjectUSetPrint(this->objects, ',', output_stream);
		output_stream << "\n";
	}

	void Situation::PrintFluents(std::ostream& output_stream) const {
		output_stream << "Fluents for situation: ";
		for (const auto& f : this->relational_fluents_) {
			output_stream << f.second;
			output_stream << "\n";
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

	std::ostream& operator<< (std::ostream& stream, const Situation& sit) {
		stream << "(";
		for (size_t i = 0; i < sit.history.size(); i++) {
			stream << sit.history[i];
			if (i != sit.history.size() - 1) {
				stream << ", ";
			}
		}
		stream << ")";
		return stream;
	}

}