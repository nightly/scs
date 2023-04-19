#include "scs/SituationCalculus/action.h"

#include <string>
#include <unordered_set>

namespace scs {

	/*
	* Ctors
	*/

	Action::Action(const std::string& name) : name(name) {}

	Action::Action(const std::string& name, const std::vector<Term>& parameters) 
		: name(name), parameters(parameters) {}

	Action::Action(std::string&& name) 
		: name(std::move(name)) {}

	Action::Action(const std::string& name, std::vector<Term>&& parameters)
		: name(name), parameters(std::move(parameters)) {}

	Action::Action(std::string&& name, std::vector<Term>&& parameters)
		: name(std::move(name)), parameters(std::move(parameters)) {}

	/*
	* Cctors, Mtors
	*/

	Action::Action(const Action& other)
		: name(other.name), parameters(other.parameters) {
	}

	Action& Action::operator=(const Action& other) {
		name = other.name;
		parameters = other.parameters;
		return *this;
	}

	Action::Action(Action&& other) noexcept
		: name(std::move(other.name)), parameters(std::move(other.parameters)) {
	}

	Action& Action::operator=(Action&& other) noexcept {
		name = std::move(other.name);
		parameters = std::move(other.parameters);
		return *this;
	}

	/*
	* General
	*/

	bool Action::HasParams() const {
		return !(parameters.empty());
	}

	/*
	* Operator overloads
	*/

	std::ostream& operator<< (std::ostream& stream, const Action& action) {
		stream << action.name << "(" << TermVectorToString(action.parameters) << ")";
		return stream;
	}


	// Note this comparison operator just based on UNA not the parameters since that requires substitutions
	// which technically, if you pass the situation in as pointer, you can call FOL evaluator to do the substitutions

	bool Action::operator==(const Action& other) const {
		return (name == other.name);
	}

	bool Action::operator!=(const Action& other) const {
		return (name != other.name);
	}
}