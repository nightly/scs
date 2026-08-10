#pragma once

#include <ostream>
#include <iostream>
#include <sstream>
#include <stdexcept>
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
#include "scs/SituationCalculus/bat.h"

namespace scs {

    inline size_t EvaluatorQuantifiedVariableCount(const Formula& formula) {
        if (const auto* unary = std::get_if<Box<UnaryConnective>>(&formula)) {
            return EvaluatorQuantifiedVariableCount((*unary)->child());
        }
        if (const auto* binary = std::get_if<Box<BinaryConnective>>(&formula)) {
            return EvaluatorQuantifiedVariableCount((*binary)->lhs())
                + EvaluatorQuantifiedVariableCount((*binary)->rhs());
        }
        if (const auto* quantifier = std::get_if<Box<Quantifier>>(&formula)) {
            return 1 + EvaluatorQuantifiedVariableCount((*quantifier)->child());
        }
        return 0;
    }

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
				throw std::invalid_argument("Variable does not denote a Boolean value");
            }
            return false;
        }

        bool operator()(const Object& o) {
            SCS_CRITICAL("[FOL] Invalid call, trying to evaluate object {} to boolean", o.name());
			throw std::invalid_argument("Object cannot be evaluated as a Boolean formula");
            return false;
        }

        bool operator()(const Action& a) {
            SCS_CRITICAL("[FOL] Invalid call, trying to evaluate action {} to boolean", a);
			throw std::invalid_argument("Action cannot be evaluated as a Boolean formula");
            return false;
        }

		bool operator()(const CompoundAction& action) {
			std::ostringstream message;
			message << "A compound action cannot be evaluated as a Boolean formula: " << action;
			throw std::invalid_argument(message.str());
		}

        bool operator()(const Situation& s) {
            SCS_CRITICAL("[FOL] Invalid call, trying to evaluate situation {} to boolean", s);
			throw std::invalid_argument("Situation cannot be evaluated as a Boolean formula");
            return false;
        }

		bool operator()(const Predicate& pred) {
			if (pred.name() == "@identifier") {
				if (pred.terms().size() != 1) {
					throw std::invalid_argument("@identifier expects exactly one term");
				}
				const Term& term = pred.terms().front();
				if (const auto* object = std::get_if<Object>(&term)) return object->IsIdentifier();
				return assignment.GetObject(std::get<Variable>(term)).IsIdentifier();
			}
			const RelationalFluent* rf_ptr = nullptr;
			if (domain.situation != nullptr && domain.situation->relational_fluents_.contains(pred.name())) {
				rf_ptr = &domain.situation->relational_fluents_.at(pred.name());
			} else if (domain.bat != nullptr && domain.bat->rigid.contains(pred.name())) {
				const auto& relation = domain.bat->rigid.at(pred.name());
				std::vector<Object> params;
				for (const auto& term : pred.terms()) {
					if (const auto* object = std::get_if<Object>(&term)) params.emplace_back(*object);
					else params.emplace_back(assignment.GetObject(std::get<Variable>(term)));
				}
				return relation.Valuation(params);
			}
			if (rf_ptr == nullptr) {
				throw std::invalid_argument("Unknown relational symbol '" + pred.name() + "'");
			}
			const RelationalFluent& rf = *rf_ptr;
            if (rf.Arity() == 0) {
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
            for (const Object& o : domain.Objects()) {
				auto extended = assignment;
				extended.Set(q->variable(), o);
				bool evaluate = std::visit(Evaluator{domain, extended}, q->child());
                if (evaluate) {
                    return true;
                }
            }
            return false;
        }

        bool EvaluateUniversal(const Box<Quantifier>& q) {
            for (const Object& o : domain.Objects()) {
				auto extended = assignment;
				extended.Set(q->variable(), o);
				bool evaluate = std::visit(Evaluator{ domain, extended }, q->child());
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
			   throw std::invalid_argument("Unsupported term in object equality");
               return nullptr;
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

		bool EquateCompoundActions(const CompoundAction& lhs, const CompoundAction& rhs) const {
			if (lhs.Actions().size() != rhs.Actions().size()) return false;
			for (size_t i = 0; i < lhs.Actions().size(); ++i) {
				if (!EquateActions(lhs.Actions()[i], rhs.Actions()[i])) return false;
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
                        return false;
                    }
                } else {
                    // <obj, unknown formula>
                    SCS_CRITICAL("[FOL] Performing equality check on non-term!");
					throw std::invalid_argument("Object equality expects object terms");
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
                        return false;
                    }
                } else if (const scs::Variable* rhs_ptr = std::get_if<Variable>(&rhs)) {
                    // <var, var>
                    return assignment.Get(*lhs_ptr) == assignment.Get(*rhs_ptr);
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
                        return false;
                    }
				} else if (const auto* rhs_ptr = std::get_if<CompoundAction>(&rhs)) {
					const auto& value = assignment.Get(*lhs_ptr);
					if (const auto* compound = std::get_if<CompoundAction>(&value)) {
						return EquateCompoundActions(*compound, *rhs_ptr);
					}
					if (const auto* action = std::get_if<Action>(&value)) {
						return EquateCompoundActions(CompoundAction{*action}, *rhs_ptr);
					}
					return false;
                } else {
                    // <var, unknown>
                    SCS_CRITICAL("[FOL] Performing equality check on non-term!");
					throw std::invalid_argument("Variable equality expects compatible terms");
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
                        return false;
                    }
                } else {
                    // <var, unknown>
                    SCS_CRITICAL("[FOL] Performing equality check on action against non-action!");
					throw std::invalid_argument("Action equality expects compatible actions");
                    return false;
                }
			} else if (const auto* lhs_ptr = std::get_if<CompoundAction>(&lhs)) {
				if (const auto* rhs_ptr = std::get_if<CompoundAction>(&rhs)) {
					return EquateCompoundActions(*lhs_ptr, *rhs_ptr);
				}
				if (const auto* rhs_ptr = std::get_if<Variable>(&rhs)) {
					const auto& value = assignment.Get(*rhs_ptr);
					if (const auto* compound = std::get_if<CompoundAction>(&value)) {
						return EquateCompoundActions(*lhs_ptr, *compound);
					}
					if (const auto* action = std::get_if<Action>(&value)) {
						return EquateCompoundActions(*lhs_ptr, CompoundAction{*action});
					}
				}
				return false;
            } else {
                // Anything else, which isn't supported.
                SCS_CRITICAL("[FOL] Performing equality check on non-term!");
				throw std::invalid_argument("Equality expects object or action terms");
                return false;
            }
        }

	};

	inline bool EvaluateFormula(const Formula& formula, Domain domain,
		const FirstOrderAssignment& assignment = {}) {
                domain = domain.WithAnonymousIdentifiers(EvaluatorQuantifiedVariableCount(formula));
		return std::visit(Evaluator{std::move(domain), assignment}, formula);
	}

}

// @Future:
// Note that pattern matching is much nicer or some kind of unification based on sorts.
