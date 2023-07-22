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
#include "scs/FirstOrderLogic/assignment.h"
#include "scs/Memory/box.h"

namespace scs {

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


        bool operator()(bool b) {
            return b ? true : false;
        }

        bool operator()(const Variable& v) {
            if (auto ptr = std::get_if<bool>(&assignment.Get(v))) {
                return *ptr;
            } else {
                SCS_CRITICAL("[FOL] Invalid call, trying to evaluate variable {} to boolean. Variable doesn't map to boolean", v.name());
                SCS_CRITICAL(assignment);
                std::exit(36);
            }
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

        bool operator()(const CoopPredicate& mx) {
            assert(domain.mat != nullptr && "Domain's CoopMatrix is nullptr but trying to check a CoopMatrixPredicate");
            std::size_t i, j;
            const auto& i_variant = assignment.Get(mx.i);
            auto i_get = std::get_if<Object>(&i_variant);

            const auto& j_variant = assignment.Get(mx.j);
            auto j_get = std::get_if<Object>(&j_variant);
            if (!i_get || !j_get) {
                SCS_TRACE("Getting i or j for CoopMatrix resulted in none object type");
                return false;
            }

            std::istringstream ss_i(*i_get), ss_j(*j_get);
            ss_i >> i;
            ss_j >> j;
            if (ss_i.fail() || ss_j.fail()) {
                SCS_TRACE("{} or {} is not a number for CoopMx", *i_get, *j_get);
                return false;
            }
            return domain.mat->Lookup(i, j);
        }

        bool operator()(const RoutePredicate& rp) {
            assert(domain.routes != nullptr && "Domain's routes is nullptr but trying to check a RoutePredicate");
            return domain.routes->Lookup(rp.i, rp.j);
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
                assignment.Set(q->variable(), o);
                bool evaluate = std::visit(Evaluator{ domain, assignment }, q->child());
                if (!evaluate) {
                    return false;
                }
            }
            return true;
        }

        // Get object from term, where the term may be either variable or object
        const scs::Object* GetObjectFromTerm(const Term& t) const {
           if (auto ptr = std::get_if<Object>(&t)) {
                return ptr;
           } else if (auto ptr = std::get_if<Variable>(&t)) {
                return &std::get<Object>(assignment.Get(*ptr)); // assume never called with action/situation
           } else {
               SCS_CRITICAL("Unsupported term (e.g. action/situation) tried to get object from"); 
           }

        }

        // Unifies between two vectors of variant<object, variable>
        bool UnifyObjects(const std::vector<Term>& t1, const std::vector<Term>& t2) const {
            for (size_t i = 0; i < t1.size(); ++i) {
                const Object* lhs = GetObjectFromTerm(t1[i]);
                const Object* rhs = GetObjectFromTerm(t2[i]);

                if (*rhs != *lhs) {
                    return false;
                }
            }
            return true;
        }

        bool EquateActions(const Action& a1, const Action& a2) const {
            if (a1.name != a2.name) {
                return false;
            }
            if (a1.terms.size() != a2.terms.size()) {
                SCS_CRITICAL("Comparing actions of the same name but with different number of terms, {} and {}", a1, a2);
                return false;
            }
            SCS_TRACE("Actions {} and {} equality = {}. Where var assignment = {}", a1, a2, UnifyObjects(a1.terms, a2.terms), assignment);
            return UnifyObjects(a1.terms, a2.terms);
        }

        bool EquateActions(const CompoundAction& ca, const Action& a) const {
            for (const auto& act : ca.Actions()) {
                if (EquateActions(act, a)) {
                    return true;
                }
            }
            return false;
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
                    // var can also be CompoundAction
                    const auto& get = assignment.Get(*lhs_ptr);
                    if (const scs::Action* get_ptr = std::get_if<Action>(&get)) {
                        return EquateActions(*get_ptr, *rhs_ptr);
                    } else if (auto get_ptr = std::get_if<CompoundAction>(&get)) {
                        return EquateActions(*get_ptr, *rhs_ptr);
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
                    return EquateActions(*lhs_ptr, *rhs_ptr);
                } else if (const scs::Variable* rhs_ptr = std::get_if<Variable>(&rhs)) {
                    // <action, var>
                    // var can also be CompoundAction
                    const auto& get = assignment.Get(*rhs_ptr);
                    if (const scs::Action* get_ptr = std::get_if<Action>(&get)) {
                        return EquateActions(*lhs_ptr, *get_ptr);
                    } else if (auto get_ptr = std::get_if<CompoundAction>(&get)) {
                        return EquateActions(*get_ptr, *lhs_ptr);
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
// Note that a variable can actually hold three sorts, situations also, which can also technically be quantified over.
// Note that pattern matching is much nicer or some kind of unification based on sorts.