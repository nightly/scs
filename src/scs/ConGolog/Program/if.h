#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"

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

        virtual void Decompose(Execution& exec) const override {
            Execution e1;
            e1.trace.Add(check);
            p->Decompose(e1);
            exec.sub_executions.emplace_back(e1);

            Execution e2;
            e2.trace.Add(UnaryConnective(check, UnaryKind::Negation));
            q->Decompose(e2);
            exec.sub_executions.emplace_back(e2);
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
