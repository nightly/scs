#pragma once

#include "scs/FirstOrderLogic/formula.h"

namespace scs {

	enum class BinaryKind : size_t {
		Conjunction,
		Disjunction,
		Implication,
		Equivalence, // Material Equivalence
		Equal,
		NotEqual,
	};

	/*
	 * @brief Higher value = higher precedence
	 */
	constexpr size_t Precedence(BinaryKind k) {
		switch (k) {
		case BinaryKind::Conjunction:
			return 9;

		case BinaryKind::Disjunction:
			return 8;

		case BinaryKind::Implication:
			return 7;

		case BinaryKind::Equivalence:
			return 6;

		case BinaryKind::Equal:
			return 5;

		case BinaryKind::NotEqual:
			return 5;
		}
	}

	class BinaryConnective {
	private:
		Formula lhs_;
		Formula rhs_;
		BinaryKind kind_;
	public:
		BinaryConnective(const Formula& lhs, const Formula& rhs, BinaryKind kind)
			: lhs_(std::move(lhs)), rhs_(std::move(rhs)), kind_(kind) {}

		BinaryKind kind() const {
			return kind_;
		}
		void set_kind(BinaryKind k) {
			kind_ = k;
		}

		const Formula& lhs() const {
			return lhs_;
		}
		Formula& lhs() {
			return lhs_;
		}
		const Formula& rhs() const {
			return rhs_;
		}
		Formula& rhs() {
			return rhs_;
		}

		bool operator==(const BinaryConnective& other) const {
			return (lhs_ == other.lhs_) && (rhs_ == other.rhs_) && (kind_ == other.kind_);
		}

		bool operator!=(const BinaryConnective& other) const {
			return (lhs_ != other.lhs_) || (rhs_ != other.rhs_) || (kind_ != other.kind_);
		}
	};



}