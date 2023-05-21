#pragma once

#include <vector>
#include <string>
#include <variant>

#include "scs/FirstOrderLogic/object.h"
#include "scs/FirstOrderLogic/unary.h"
#include "scs/FirstOrderLogic/binary.h"
#include "scs/FirstOrderLogic/quantifier.h"
#include "scs/Memory/box.h"

namespace scs {

    struct SuccessorActionExtractor {
    public:
        std::unordered_set<std::string> actions_;
    public:
        const auto& Actions() const { return actions_;  }
        void Reset() { actions_.clear(); }
    public:

        void operator()(const auto& arbitary) {
            // Falls under variable, object, boolean, situation, predicates
            return;
        }

        void operator()(const Action& a) {
            actions_.emplace(a.name);
        }

        void operator()(const Box<UnaryConnective>& u) {
            std::visit(*this, u->child());
        }

        void operator()(const Box<BinaryConnective>& c) {
            std::visit(*this, c->lhs());
            std::visit(*this, c->rhs());
        }

        void operator()(const Box<Quantifier>& q) {
            std::visit(*this, q->child());
        }

    };


}