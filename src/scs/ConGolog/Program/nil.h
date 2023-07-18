#pragma once

#include "scs/ConGolog/Program/interface_program.h"

namespace scs {

	// Abbreviates = (True?)
	struct Nil : public IProgram {
	public:
		Nil() {}

		std::shared_ptr<IProgram> clone() const override {
			return std::make_shared<Nil>(*this);
		}

		virtual void AddTransition(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker,
		std::optional<std::shared_ptr<CgTransition>> transition_opt = std::nullopt) const override {

		}

		virtual ProgramStep Step(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker,
		std::optional<std::shared_ptr<CgTransition>> transition_opt = std::nullopt) const override {
			return {};
		}

		std::ostream& Print(std::ostream& os) const override {
			os << "<Nil> ";
			return os;
		}

		virtual bool operator==(const Nil& nil) const override {
			return true;
		}
	};

	inline std::ostream& operator<< (std::ostream& os, const Nil& nil) {
		nil.Print(os);
		return os;
	}

}