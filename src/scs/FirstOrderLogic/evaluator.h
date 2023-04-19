#pragma once

#include <ostream>
#include <iostream>
#include <variant>

#include "matchit.h"

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
        std::unordered_map<scs::Variable, std::variant<scs::Object, scs::Action>> variables_map_;
        std::unordered_set<scs::Object> bound_;
    public:
        void Set(const scs::Variable& var, const scs::Object& o) {
            bound_.emplace(o);
            variables_map_[var] = o;
        }

        void Set(const scs::Variable& var, const scs::Action& a) {
            variables_map_[var] = a;
        }

        const std::variant<Object, Action>& Get(const Variable& var) {
            return variables_map_[var];
        }

        bool IsBound(const scs::Object& o) const {
            return bound_.contains(o);
        }
    };

    struct Evaluator {
    public:
        Domain domain;
        FirstOrderAssignment assignment;
    public:
        /* Constructors */
        Evaluator() {}
        Evaluator(const FirstOrderAssignment& assignment) : assignment(assignment) {}

        Evaluator(Domain d) : domain(d) {}
        Evaluator(Domain d, const FirstOrderAssignment& assignment) : domain(d), assignment(assignment) {}

        // Defined for convenience based on how we currently handle domains.
        Evaluator(const Situation& s) : domain(&s) {} 
        Evaluator(const Situation& s, const FirstOrderAssignment& assignment) : domain(&s), assignment(assignment) {}


        bool operator()(bool b) {
            return b ? true : false;
        }

        bool operator()(const Variable& v) {
            SCS_CRITICAL("[FOL] Invalid call, trying to evaluate variable {} to boolean", v.name());
            std::exit(36);
            return false;
        }

        bool operator()(const Object& o) {
            SCS_CRITICAL("[FOL] Invalid call, trying to evaluate object {} to boolean", o.name());
            std::exit(36);
            return false;
        }

        bool operator()(const Action& a) {
            SCS_CRITICAL("[FOL] Invalid call, trying to evaluate action {} to boolean", a);
            std::exit(36);
            return false;
        }

        bool operator()(const Situation& s) {
            SCS_CRITICAL("[FOL] Invalid call, trying to evaluate situation {} to boolean", s);
            std::exit(36);
            return false;
        }

        bool operator()(const Predicate& pred) {
            const RelationalFluent& rf = domain.situation->relational_fluents_.at(pred.name());
            if (rf.Is0Arity()) {
                return rf.Valuation();
            } else {
                std::vector<scs::Object> params;
                for (const auto& p : pred.terms_) {
                    if (const scs::Object* obj_ptr = std::get_if<Object>(&p)) {
                        params.emplace_back(*obj_ptr);
                    } else if (const scs::Variable* var_ptr = std::get_if<Variable>(&p)) {
                        // Because we also need to deal with variant
                        const auto& get = assignment.Get(*var_ptr);
                        if (const scs::Object* get_ptr = std::get_if<Object>(&get)) {
                            params.emplace_back(*get_ptr);
                        } else {
                            SCS_CRITICAL("[FOL] Trying to emplace a variable that maps to non-object in relational fluent valuation!");
                        }
                    } else {
                        SCS_CRITICAL("[FOL] Unsupported data type in predicate parameter evaluation");
                        return false;
                    }
                }
                return rf.Valuation(params);
            }
        }

        bool operator()(const Box<UnaryConnective>& u) {
            if (u->kind() == UnaryKind::Negation) {
                auto result = std::visit(*this, u->child());
                return !result;
            } else {
                SCS_CRITICAL("[FOL] Unimplemented unary operator!");
                return false;
            }
        }

        bool operator()(const Box<BinaryConnective>& c) {
            auto kind = c->kind();
            if (kind == BinaryKind::Conjunction) {
                return std::visit(*this, c->lhs()) && std::visit(*this, c->rhs());;
            } else if (kind == BinaryKind::Disjunction) {
                return std::visit(*this, c->lhs()) || std::visit(*this, c->rhs());;
            } else if (kind == BinaryKind::Implication) {
                return !std::visit(*this, c->lhs()) || std::visit(*this, c->rhs());;
            } else if (kind == BinaryKind::Equivalence) {
                return std::visit(*this, c->lhs()) == std::visit(*this, c->rhs());;
            } else if (kind == BinaryKind::Equal) {
                return EvaluateEquality(c);
            } else if (kind == BinaryKind::NotEqual) {
                return !EvaluateEquality(c);
            } else {
                SCS_CRITICAL("[FOL] Unimplemented binary operator!");
                return false;
            }
        }

        bool operator()(const Box<Quantifier>& q) {
            if (q->kind() == QuantifierKind::Universal) {
                return EvaluateUniversal(q);
            } else if (q->kind() == QuantifierKind::Existential) {
                return EvaluateExistential(q);
            } else {
                SCS_CRITICAL("[FOL] Unimplemented quantifier type!");
                return false;
            }
        }

        bool EvaluateExistential(const Box<Quantifier>& q) {
            for (const Object& o : domain.situation->objects) {
                if (assignment.IsBound(o)) {
                    continue; // This object is already binded to another variable
                }
                assignment.Set(q->variable(), o);
                bool evaluate = std::visit(Evaluator{domain, assignment}, q->child());
                if (evaluate) {
                    return true;
                }
            }
            return false;
        }

        bool EvaluateUniversal(const Box<Quantifier>& q) {
            for (const Object& o : domain.situation->objects) {
                if (assignment.IsBound(o)) {
                    continue; // This object is already binded to another variable
                }
                assignment.Set(q->variable(), o);
                bool evaluate = std::visit(Evaluator{ domain, assignment }, q->child());
                if (!evaluate) {
                    return false;
                }
            }
            return true;
        }

        bool EvaluateEquality(const Box<BinaryConnective>& c) {
            // This should really use pattern-matching instead...

            // Equality may only apply to objects, or terms. In fact, this probably should bind to a Term instead and use the Term class.
            const auto& lhs = c->lhs();
            const auto& rhs = c->rhs();

            if (const scs::Object* lhs_ptr = std::get_if<Object>(&lhs)) {
                // LHS is object.
                if (const scs::Object* rhs_ptr = std::get_if<Object>(&rhs)) {
                    // <obj, obj>
                    return *lhs_ptr == *rhs_ptr;
                } else if (const scs::Variable* rhs_ptr = std::get_if<Variable>(&rhs)) {
                    // <obj, var>
                    const auto& get = assignment.Get(*rhs_ptr);
                    if (const scs::Object* get_ptr = std::get_if<Object>(&get)) {
                        return *lhs_ptr == *get_ptr;
                    } else {
                        SCS_CRITICAL("[FOL] Performing equality check on object (LHS) against variable not mapped to object!");
                    }
                } else {
                    // <obj, unknown formula>
                    SCS_CRITICAL("[FOL] Performing equality check on non-term!");
                    std::exit(72);
                    return false;
                }
            } else if (const scs::Variable* lhs_ptr = std::get_if<Variable>(&lhs)) {
                // LHS is variable.
                if (const scs::Object* rhs_ptr = std::get_if<Object>(&rhs)) {
                    // <var, obj>
                    const auto& get = assignment.Get(*lhs_ptr);
                    if (const scs::Object* get_ptr = std::get_if<Object>(&get)) {
                        return *get_ptr == *rhs_ptr;
                    } else {
                        SCS_CRITICAL("[FOL] Equality checking var LHS does not map to RHS object!");
                    }                   
                } else if (const scs::Variable* rhs_ptr = std::get_if<Variable>(&rhs)) {
                    // <var, var>
                    return *lhs_ptr == *rhs_ptr;
                } else if (const scs::Action* rhs_ptr = std::get_if<Action>(&rhs)) {
                    // <var, action>
                    const auto& get = assignment.Get(*lhs_ptr);
                    if (const scs::Action* get_ptr = std::get_if<Action>(&get)) {
                        return *get_ptr == *rhs_ptr;
                    } else {
                        SCS_CRITICAL("[FOL] Equality checking on var LHS not same type as action RHS");
                    }
                } else {
                    // <var, unknown>
                    SCS_CRITICAL("[FOL] Performing equality check on non-term!");
                    std::exit(72);
                    return false;
                }
            } else if (const scs::Action* lhs_ptr = std::get_if<Action>(&lhs)) {
                // LHS is action.
                if (const scs::Action* rhs_ptr = std::get_if<Action>(&rhs)) {
                    // <action, action>
                    return *lhs_ptr == *rhs_ptr;
                } else if (const scs::Variable* rhs_ptr = std::get_if<Variable>(&rhs)) {
                    // <action, var>
                    const auto& get = assignment.Get(*rhs_ptr);
                    if (const scs::Action* get_ptr = std::get_if<Action>(&get)) {
                        return *lhs_ptr == *get_ptr;
                    } else {
                        SCS_CRITICAL("[FOL] Performing equality check on object (LHS) against variable not mapped to object!");
                    }
                } else {
                    // <var, unknown>
                    SCS_CRITICAL("[FOL] Performing equality check on action against non-action!");
                    std::exit(72);
                    return false;
                }
            } else {
                // Anything else, which isn't supported.
                SCS_CRITICAL("[FOL] Performing equality check on non-term!");
                std::exit(73);
            }
        }

    };

}

// Is it possible to short-circuit such that when evaluating a Predicate, with objects that are not contained in a valid RelationalFluent valuation at all,
// we do not iterate over that object at all and move to the next object?

// @Future:
// Note that a variable can actually hold three sorts, not just domain objects.
// Note that pattern matching is much nicer or some kind of unification based on sorts.