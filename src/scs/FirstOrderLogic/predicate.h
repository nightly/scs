#pragma once

#include <vector>
#include <string>

#include "scs/FirstOrderLogic/term.h"
#include "scs/FirstOrderLogic/object.h"
#include "scs/FirstOrderLogic/variable.h"

namespace scs {

	/*
	* @brief: Predicate represents a helper class for a RelationalFluent, just that it supports having variables that must be binded,
	* whereas RelationalFluent has concrete valuations stored only. There's a 1-1 mapping based on name.
	* Note that, successor state axioms and Poss (precondition) axioms should be interpreted as an entire Formula provided, not as predicates themselves.
	*/
	struct Predicate {
		std::string name_;
		std::vector<Term> terms_;

		Predicate(const std::string& name) : name_(name) {}
		Predicate(std::string&& name) : name_(std::move(name)) {}

		Predicate(const std::string& name, const std::vector<Term>& terms) : name_(name), terms_(terms) {}
		Predicate(const std::string& name, std::vector<Term>&& terms) : name_(name), terms_(std::move(terms)) {}
		Predicate(std::string&& name, std::vector<Term>&& terms) : name_(std::move(name)), terms_(std::move(terms)) {}
		
		const std::string& name() const { return name_; }
		const std::vector<Term>& terms() const { return terms_; }
		bool operator==(const Predicate& other) const {
			return (name_ == other.name_) && (terms_ == other.terms_);
		}

		bool operator!=(const Predicate& other) const {
			return (name_ != other.name_) || (terms_ != other.terms_);
		}
	};

	inline std::ostream& operator<< (std::ostream& stream, const Predicate& predicate) {
		stream << predicate.name_ << "(";
		for (size_t i = 0; i < predicate.terms_.size(); ++i) {
			stream << predicate.terms_[i];
			if (i != predicate.terms_.size() - 1) {
				stream << ", ";
			}
		}
		stream << ")";
		return stream;
	}

}

	/*
	 * Technically, there could also be std::function<bool(std::initializer_list<Term> parameters)> in FOL, which could be a variant of type RelationalFluent,
	 * i.e. those controlled by SSAs and those controlled by normal calls since SSAs involve updating things that aren't even used.
	 * Don't think there's any difference between them and, generally, only relational fluents are used afaik.
	 */

