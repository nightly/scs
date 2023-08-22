#pragma once

#include <string>

#include "scs/FirstOrderLogic/forward.h"
#include "scs/FirstOrderLogic/object.h"
#include "scs/FirstOrderLogic/variable.h"
#include "scs/FirstOrderLogic/formula.h"

namespace scs {

	enum class QuantifierKind : size_t {
		Universal,
		Existential
	};

	class Quantifier {
	private:
		Variable var_;
		Formula child_;
		QuantifierKind kind_;
	public:
		Quantifier(const Variable& var, const Formula& child, QuantifierKind kind)
			:  var_(var), child_(child), kind_(kind) {}
		~Quantifier() = default;

		/* Getters and setters */
		QuantifierKind kind() const { return kind_; }
		void set_kind(QuantifierKind kind) { kind_ = kind; }

		const Variable& variable() const { return var_; }
	
		const Formula& child() const { return child_; }
		Formula& child() { return child_; }

		/* Operator overloads */
		bool operator==(const Quantifier& other) const {
			return (child_ == other.child_) && (kind_ == other.kind_) && (var_ == other.var_);
		}

		bool operator!=(const Quantifier& other) const {
			return (child_ != other.child_) || (kind_ != other.kind_) || (var_ != other.var_);
		}
	};

}