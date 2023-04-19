#pragma once

#include <memory>

#include "scs/FirstOrderLogic/formula.h"
#include "scs/Common/log.h"

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
		UnaryConnective(Formula f, UnaryKind k)
			: child_(std::move(f)), kind_(k) {
		}

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

		bool operator==(const UnaryConnective& other) const {
			return (child_ == other.child_) && (kind_ == other.kind_);
		}

		bool operator!=(const UnaryConnective& other) const {
			return (child_ != other.child_) || (kind_ != other.kind_);
		}

	};

}
