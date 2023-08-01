#include "scs/SituationCalculus/action.h"

#include <string>

namespace scs {

	/*
	* Ctors
	*/

	Action::Action(const std::string& name) : name(name) {}

	Action::Action(const std::string& name, const std::vector<Term>& terms) 
		: name(name), terms(terms) {}

	Action::Action(std::string&& name) 
		: name(std::move(name)) {}

	Action::Action(const std::string& name, std::vector<Term>&& terms)
		: name(name), terms(std::move(terms)) {}

	Action::Action(std::string&& name, std::vector<Term>&& terms)
		: name(std::move(name)), terms(std::move(terms)) {}

	/*
	* Cctors, Mtors
	*/

	Action::Action(const Action& other)
		: name(other.name), terms(other.terms) {
	}

	Action& Action::operator=(const Action& other) {
		name = other.name;
		terms = other.terms;
		return *this;
	}

	Action::Action(Action&& other) noexcept
		: name(std::move(other.name)), terms(std::move(other.terms)) {
	}

	Action& Action::operator=(Action&& other) noexcept {
		name = std::move(other.name);
		terms = std::move(other.terms);
		return *this;
	}

	/*
	* General
	*/

	bool Action::HasParams() const {
		return !(terms.empty());
	}

	/*
	* Operator overloads
	*/

	std::ostream& operator<< (std::ostream& stream, const Action& action) {
		stream << action.name << "(" << TermVectorToString(action.terms) << ")";
		return stream;
	}

	// Use EquateActions() in FOL equality to resolve terms, this is term and name-wise
	// So, var(x) and obj(...) are not equal, even though they could be in some assignment
	bool Action::operator==(const Action& other) const {
		if (name != other.name || terms.size() != other.terms.size()) {
			return false;
		}
		for (size_t i = 0; i < terms.size(); ++i) {
			if (terms[i] != other.terms[i]) {
				return false;
			}
		}
		return true;
	}

	bool Action::operator!=(const Action& other) const {
		return !(*this == other);
	}
}