#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"

#include "scs/Memory/box.h"

namespace scs {

	// Non-deterministic branching (p | q)
	struct Branch : public IProgram {
		std::shared_ptr<IProgram> p;
		std::shared_ptr<IProgram> q;

		template<typename P, typename Q>
		Branch(const P* p, const Q* q)
			: p(std::make_shared<P>(*p)), q(std::make_shared<Q>(*q)) {}

		template<typename P, typename Q>
		Branch(const P& p, const Q& q)
			: p(std::make_shared<P>(p)), q(std::make_shared<Q>(q)) {}


		virtual void Decompose(Execution& exec) const override {
			Execution e1;
			p->Decompose(e1);
			exec.sub_executions.emplace_back(e1);

			Execution e2;
			q->Decompose(e2);
			exec.sub_executions.emplace_back(e2);
		}


		bool Final(const Situation& s) const override {
			return p->Final(s) || q->Final(s);
		}


		std::ostream& Print(std::ostream& os) const override {
			os << "<NonDet>" << *p << " | " << *q;
			os << "\n";
			return os;
		}

	};

	inline std::ostream& operator<< (std::ostream& os, const Branch& br) {
		br.Print(os);
		return os;
	}

}