#include "scs/SituationCalculus/situation.h"

#include <vector>
#include <string>

#include "scs/SituationCalculus/action.h"
#include "scs/Common/log.h"

#include "scs/SituationCalculus/poss.h"
#include "scs/SituationCalculus/successor.h"
#include "scs/FirstOrderLogic/evaluator.h"

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

	size_t Situation::Length() const {
		return history.size();
	}

	bool Situation::Possible(const Action& a, const Poss& poss) const {
		FirstOrderAssignment assignment;

		for (size_t i = 0; i < a.parameters.size(); ++i) {
			const auto& obj = std::get<Object>(a.parameters[i]); // Performing an action, must have complete object literals
			if (const scs::Variable* var_ptr = std::get_if<Variable>(&poss.Terms().at(i))) {
				assignment.Set(*var_ptr, obj);
			}
		}
		scs::Evaluator eval{ *this, assignment };
		return std::visit(eval, poss.Form());
	}

	Situation Situation::Do(const Action& a, const Poss& poss, const Successor& ssa) const {
		return Situation{};
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