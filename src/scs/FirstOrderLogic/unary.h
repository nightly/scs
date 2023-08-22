#pragma once

#include <memory>

#include "scs/Common/log.h"
#include "scs/FirstOrderLogic/formula.h"

namespace scs {

	enum class UnaryKind : size_t {
		Negation,
	};

	constexpr size_t Precedence(UnaryKind k) {
		switch (k) {
		case UnaryKind::Negation:
			return 10;
		}
	}

	class UnaryConnective {
	private:
		Formula child_;
		UnaryKind kind_;
	public:
		UnaryConnective(const Formula& f, UnaryKind k)
			: child_(f), kind_(k) {
		}
		~UnaryConnective() = default;

		/* Getters and setters */
		UnaryKind kind() const {
			return kind_;
		}
		void set_kind(UnaryKind k) {
			kind_ = k;
		}

		const Formula& child() const {
			return child_;
		}
		Formula& child() {
			return child_;
		}

		/* Operator overloads */
		bool operator==(const UnaryConnective& other) const {
			return (child_ == other.child_) && (kind_ == other.kind_);
		}

		bool operator!=(const UnaryConnective& other) const {
			return (child_ != other.child_) || (kind_ != other.kind_);
		}

	};

}
