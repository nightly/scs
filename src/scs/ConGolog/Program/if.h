#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"
#include "scs/ConGolog/Program/nil.h"

namespace scs {

    struct CgIf : public IProgram {
    public:
        Formula check;
        std::shared_ptr<IProgram> p; // true block
        std::shared_ptr<IProgram> q; // else block

    public:
        CgIf(const Formula& check, const IProgram* p, const IProgram* q)
            : check(check), p(p->clone()), q(q->clone()) {}

        CgIf(const Formula& check, const IProgram& p, const IProgram& q)
            : check(check), p(p.clone()), q(q.clone()) {}

        std::shared_ptr<IProgram> clone() const override {
            return std::make_shared<CgIf>(*this);
        }

        virtual void AddTransition(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker,
        std::optional<std::shared_ptr<CgTransition>> transition_opt = std::nullopt) const override {
            StateTracker t1(tracker), t2(tracker);

            auto true_transition = std::make_shared<CgTransition>();
            true_transition->condition = check;
            p->AddTransition(graph, counter, t1, true_transition);

            auto else_transition = std::make_shared<CgTransition>();
            else_transition->condition = UnaryConnective(check, UnaryKind::Negation);
            q->AddTransition(graph, counter, t2, else_transition);

            tracker = t1 + t2;
        }

        virtual std::shared_ptr<IProgram> Step(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker,
        std::optional<std::shared_ptr<CgTransition>> transition_opt = std::nullopt) const override {
            return std::make_shared<Nil>();
        }

        std::ostream& Print(std::ostream& os) const override {
            os << "<If>" << " " << (check) << '\n';
            os << "	<Then>";
            os << *(p.get());
            os << "	<Else>";
            os << *(q.get());
            os << "\n";
            return os;
        }


    };

    inline std::ostream& operator<< (std::ostream& os, const CgIf& prog) {
        prog.Print(os);
        return os;
    }


}
