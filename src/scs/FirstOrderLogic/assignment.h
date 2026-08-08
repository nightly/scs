#pragma once

#include <ostream>
#include <variant>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stdexcept>

#include <spdlog/fmt/ostr.h>

#include "scs/FirstOrderLogic/object.h"
#include "scs/FirstOrderLogic/unary.h"
#include "scs/FirstOrderLogic/binary.h"
#include "scs/FirstOrderLogic/quantifier.h"
#include "scs/FirstOrderLogic/symbols.h"
#include "scs/FirstOrderLogic/domain.h"
#include "scs/Memory/box.h"

namespace scs {

    struct FirstOrderAssignment {
    public:
        using Value = std::variant<scs::Object, Action, CompoundAction, bool>;
        using Map = std::unordered_map<scs::Variable, Value>;
    private:
        Map variables_map_;
    public:
        void Set(const scs::Variable& var, const scs::Object& o) {
            variables_map_.insert_or_assign(var, o);
        }
        void Set(const scs::Variable& var, const scs::Action& a) {
            variables_map_.insert_or_assign(var, a);
        }        
        void Set(const scs::Variable& var, const scs::CompoundAction& ca) {
            variables_map_.insert_or_assign(var, ca);
        }
        void Set(const scs::Variable& var, bool b) {
            variables_map_.insert_or_assign(var, b);
        }

        bool Contains(const Variable& var) const {
            return variables_map_.contains(var);
        }

        const Value& Get(const Variable& var) const {
            if (!variables_map_.contains(var)) {
                throw std::runtime_error("Variables map does not contain the search variable: " + var.name());
            }
            return variables_map_.at(var);
        }

        const Object& GetObject(const Variable& var) const {
            const auto* object = std::get_if<Object>(&Get(var));
            if (object == nullptr) {
                throw std::invalid_argument("Variable '" + var.name() + "' is not bound to an object");
            }
            return *object;
        }

        const Map& Values() const {
            return variables_map_;
        }

        Map::const_iterator begin() const { return variables_map_.begin(); }
        Map::const_iterator end() const { return variables_map_.end(); }

        FirstOrderAssignment Extended(const Variable& var, const Object& object) const {
            FirstOrderAssignment result = *this;
            result.Set(var, object);
            return result;
        }

        FirstOrderAssignment Project(const std::vector<Variable>& variables) const {
            FirstOrderAssignment result;
            for (const auto& variable : variables) {
                if (const auto found = variables_map_.find(variable); found != variables_map_.end()) {
                    result.variables_map_.emplace(found->first, found->second);
                }
            }
            return result;
        }

        bool IsBound(const scs::Object& o) const {
            for (const auto& [variable, value] : variables_map_) {
                (void)variable;
                if (const auto* object = std::get_if<Object>(&value); object != nullptr && *object == o) {
                    return true;
                }
            }
            return false;
        }

        bool operator==(const FirstOrderAssignment& other) const {
            return variables_map_ == other.variables_map_;
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
        std::unordered_set<Object> objects;
        for (const auto& [variable, value] : assignment.variables_map_) {
            (void)variable;
            if (const auto* object = std::get_if<Object>(&value); object != nullptr && objects.emplace(*object).second) {
                os << *object << ",";
            }
        }
        os << ")";
        return os;
    }

}

template <>
struct fmt::formatter<scs::FirstOrderAssignment> : fmt::ostream_formatter {};
