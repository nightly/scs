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
        template<typename P, typename Q>
        CgIf(const Formula& check, const P* p, const Q* q)
            : check(check), p(std::make_shared<P>(*p)), q(std::make_shared<Q>(*q)) {}

        template<typename P, typename Q>
        CgIf(const Formula& check, const P& p, const Q& q)
            : check(check), p(std::make_shared<P>(p)), q(std::make_shared<Q>(q)) {}


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

        bool Final(const Situation& s) const override {
            return false;
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
