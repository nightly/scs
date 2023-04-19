#pragma once

namespace scs {

	enum class Turn {
		Controller,
		Environment,
	};

	void EvolveTurn(Turn& t);

}