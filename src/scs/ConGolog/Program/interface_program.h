#pragma once

#include <vector>

#include "scs/SituationCalculus/bat.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/ConGolog/CharacteristicGraph/utils/utils.h"

namespace scs {

	struct Nil;

	struct ProgramStep {
		std::shared_ptr<IProgram> evolved_program;
		std::optional<std::shared_ptr<CgTransition>> evolved_transition = std::nullopt;
	};

	struct IProgram {
		virtual ~IProgram() = default;

		virtual std::shared_ptr<IProgram> clone() const = 0;

		virtual void AddTransition(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker, 
		std::optional<std::shared_ptr<CgTransition>> transition = std::nullopt) const = 0;

		virtual ProgramStep Step(CharacteristicGraph& graph, StateCounter& counter, StateTracker& tracker,
		std::optional<std::shared_ptr<CgTransition>> transition = std::nullopt) const = 0;

		virtual bool operator==(const Nil& nil) const {
			return false;
		}

		virtual bool operator!=(const Nil& nil) const {
			return !(*this == nil);
		}

		virtual std::ostream& Print(std::ostream& os) const = 0;
	};

	inline std::ostream& operator<< (std::ostream& os, const IProgram& prog) {
		prog.Print(os);
		return os;
	}

	inline std::shared_ptr<CgTransition> GetTransition(std::optional<std::shared_ptr<CgTransition>>& opt) {
		if (opt.has_value()) {
			return *opt;
		} else {
			return std::make_shared<CgTransition>();
		}
	}

}

// virtual IProgram* clone() const = 0;
