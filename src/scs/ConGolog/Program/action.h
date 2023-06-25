#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"
#include "scs/SituationCalculus/compound_action.h"

namespace scs {


	struct ActionProgram : public IProgram {
	public:
		CompoundAction act;
	public:
		ActionProgram(const Action& act) 
			: act(act) {}

		ActionProgram(const CompoundAction& ca)
			: act(ca) {}

		std::shared_ptr<IProgram> clone() const override {
			return std::make_shared<ActionProgram>(*this);
		}

		virtual void AddTransition(CharacteristicGraph& graph, StateCounter& counter,
		StateTracker& tracker, CgTransition transition = CgTransition(), int loop_back = -1) const override {
			for (const auto& current : tracker.CurrentStates()) {
				size_t next;
				if (transition.to != 0) {
					next = counter.Increment();
				} else {
					next = transition.to;
				}
				transition.act = this->act;
				graph.lts.AddTransition(current, transition, next);
			}
		}

		std::ostream& Print(std::ostream& os) const override {
			if (act.IsSimple()) {
				os << "<Action> " << act;
			} else {
				os << "<CompoundAction> " << act;
			}
			return os;
		}

	};

	inline std::ostream& operator<< (std::ostream& os, const ActionProgram& ap) {
		ap.Print(os);
		return os;
	}

}