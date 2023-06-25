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

        virtual void AddTransition(CharacteristicGraph& graph, StateCounter& counter,
        StateTracker& tracker, CgTransition transition = CgTransition(), int loop_back = -1) const override {
        
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
