#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"
#include "scs/SituationCalculus/compound_action.h"

namespace scs {


	struct ActionProgram : public IProgram {
	public:
		Action act;
	public:
		ActionProgram(const Action& act) 
			: act(act) {}

		std::shared_ptr<IProgram> clone() const override {
			return std::make_shared<ActionProgram>(*this);
		}
		
		//virtual void Decompose(Execution& exec) const override {
		//	exec.trace.Add(act);
		//}

		std::ostream& Print(std::ostream& os) const override {
			os << "<Action> " << act;
			return os;
		}

	};

	inline std::ostream& operator<< (std::ostream& os, const ActionProgram& ap) {
		ap.Print(os);
		return os;
	}

}