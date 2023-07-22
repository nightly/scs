#pragma once

#include <ostream>
#include <iostream>
#include <variant>

#include "scs/FirstOrderLogic/object.h"
#include "scs/FirstOrderLogic/unary.h"
#include "scs/FirstOrderLogic/binary.h"
#include "scs/FirstOrderLogic/quantifier.h"
#include "scs/FirstOrderLogic/symbols.h"
#include "scs/Memory/box.h"

namespace scs {

	struct Printer {
	public:
		std::ostream& os;
		const Symbols& sym;
	public:
		Printer(std::ostream& os = std::cout, const Symbols& sym = unicode)
			: os(os), sym(sym) {}

        void operator()(bool b) {
            if (b) {
                os << "True";
            } else {
                os << "False";
            }
        }

        void operator()(const Object& obj) {
            os << "obj(" << obj.name() << ")";
        }

        void operator()(const Variable& v) {
            os << "var(" << v.name() << ")";
        }

        void operator()(const Action& a) {
            os << "act(" << a << ")";
        }

        void operator()(const Situation& s) {
            if (s.history.empty()) {
                os << "sit(initial)";
            }
            os << "sit(" << s << ")";
        }

        void operator()(const Predicate& pred) {
            os << pred;
        }

        void operator()(const CoopPredicate& coop) {
            os << coop;
        }

        void operator()(const RoutePredicate& route_pred) {
            os << route_pred;
        }

        void operator()(const Box<UnaryConnective>& u) {
            os << sym(u->kind());
            std::visit(Printer{os, sym}, u->child());
        }

        void operator()(const Box<BinaryConnective>& c) {
            os << '(';
            std::visit(Printer{os, sym}, c->lhs());
            os << ' ' << sym(c->kind()) << ' ';
            std::visit(Printer{os, sym}, c->rhs());
            os << ')';
        }

        void operator()(const Box<Quantifier>& q) {
            os << sym(q->kind()) << ' ' << q->variable().name() << ": ";
            std::visit(Printer{os, sym}, q->child());
        }

	};

}