#pragma once

#include <ostream>
#include <variant>
#include <unordered_map>

#include "scs/FirstOrderLogic/object.h"
#include "scs/FirstOrderLogic/unary.h"
#include "scs/FirstOrderLogic/binary.h"
#include "scs/FirstOrderLogic/quantifier.h"
#include "scs/FirstOrderLogic/symbols.h"
#include "scs/FirstOrderLogic/domain.h"
#include "scs/Memory/box.h"

namespace scs {

    struct FirstOrderAssignment {
    private:
        std::unordered_map<scs::Variable, std::variant<scs::Object, Action, CompoundAction, bool>> variables_map_;
        std::unordered_set<scs::Object> bound_; // @Correctness: Actually, technically in FOL, multiple variables can actually be bound to same object.
    public:
        void Set(const scs::Variable& var, const scs::Object& o) {
            bound_.emplace(o);
            variables_map_[var] = o;
        }
        void Set(const scs::Variable& var, const scs::Action& a) {
            variables_map_[var] = a;
        }        
        void Set(const scs::Variable& var, const scs::CompoundAction& ca) {
            variables_map_[var] = ca;
        }
        void Set(const scs::Variable& var, bool b) {
            variables_map_[var] = b;
        }

        const std::variant<Object, Action, CompoundAction, bool>& Get(const Variable& var) const {
            return variables_map_.at(var);
        }

        bool IsBound(const scs::Object& o) const {
            return bound_.contains(o);
        }
    public:
        friend std::ostream& operator<< (std::ostream& os, const FirstOrderAssignment& assignment);
    };

    inline std::ostream& operator<< (std::ostream& os, const FirstOrderAssignment& assignment) {
        os << "variables(";
        for (const auto& v : assignment.variables_map_) {
            os << v.first << " = ";
            if (const scs::Object* ptr = std::get_if<Object>(&v.second)) {
                os << *ptr;
            } else if (const scs::Action* ptr = std::get_if<Action>(&v.second)) {
                os << *ptr;
            } else if (auto ptr = std::get_if<CompoundAction>(&v.second)) {
                os << *ptr;
            }
            os << ",";
        }
        os << ") --- ";
        os << "bindings(";
        for (const auto& obj : assignment.bound_) {
            os << obj << ",";
        }
        os << ")";
        return os;
    }

}