#include "scs/Synthesis/Game/turn.h"

namespace scs {

	void EvolveTurn(Turn& t) {
		using enum Turn;
		switch (t) {
		case Turn::Controller:
			t = Turn::Environment;
			break;
		case Turn::Environment:
			t = Turn::Controller;
			break;
		}
	}

}