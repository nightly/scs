#pragma once

#include <string>

#include "scs/FirstOrderLogic/object.h"
#include "scs/FirstOrderLogic/unary.h"
#include "scs/FirstOrderLogic/binary.h"
#include "scs/FirstOrderLogic/quantifier.h"

#include <boost/container/flat_map.hpp>

namespace scs {

	class Symbols {
	public:
		template<typename Key, typename Value, typename Compare = std::less<Key>>
		using flat_map = boost::container::flat_map<Key, Value, Compare>;
	private:
		flat_map<UnaryKind, std::string> unary_;
		flat_map<BinaryKind, std::string> binary_;
		flat_map<QuantifierKind, std::string> quantifier_;
	public:
		Symbols(flat_map<UnaryKind, std::string> const& unary,
			   flat_map<BinaryKind, std::string> const& binary,
			   flat_map<QuantifierKind, std::string> const& quantifier)
			: unary_{ unary }, binary_{ binary }, quantifier_{ quantifier } {}

		std::string operator()(UnaryKind k) const {
			return unary_.at(k);
		}
		std::string operator()(BinaryKind k) const {
			return binary_.at(k);
		}
		std::string operator()(QuantifierKind k) const {
			return quantifier_.at(k);
		}

	};

	static const Symbols unicode = Symbols{
		{{UnaryKind::Negation,"¬"}},

		{{BinaryKind::Conjunction,"∧"},
		{BinaryKind::Disjunction,"∨"},
		{BinaryKind::Implication,"⊃"},
		{BinaryKind::Equivalence,"≡"},
		{BinaryKind::Equal,"="},
		{BinaryKind::NotEqual,"≠"}},
		
		{{QuantifierKind::Universal,"∀"}, {QuantifierKind::Existential,"∃"}}
	};

}