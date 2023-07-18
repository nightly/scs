#pragma once

#include "scs/ConGolog/Program/interface_program.h"
#include "scs/FirstOrderLogic/formula.h"
#include "scs/SituationCalculus/compound_action.h"
#include "scs/ConGolog/Program/nil.h"

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

		virtual void AddTransition(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker, 
		std::optional<std::shared_ptr<CgTransition>> transition_opt = std::nullopt) const override {
			auto transition = GetTransition(transition_opt);

			size_t next = counter.Increment();
			for (const auto& current : tracker.CurrentStates()) {
				transition->act = this->act;
				graph.lts.AddTransition(current, *transition, next);
			}
			tracker.SetState(next);
		}

		virtual ProgramStep Step(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker,
		std::optional<std::shared_ptr<CgTransition>> transition_opt = std::nullopt) const override {
			ProgramStep ret;

			auto transition = GetTransition(transition_opt);
			transition->act.AppendAction(act.Actions().at(0));
			
			ret.evolved_transition = transition;
			ret.evolved_program = std::make_shared<Nil>();
			return ret;
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