#pragma once

#include <chrono>
#include <cstddef>
#include <functional>
#include <optional>

#include "candidate.h"

namespace scs {

	enum class SynthesisStatus {
		Solved,
		NoController,
		Cancelled,
	};

	struct SynthesisStatistics {
		size_t visited_situations = 0;
		size_t action_considerations = 0;
		size_t cached_fluent_states = 0;
		size_t cache_hits = 0;
		size_t topology_states = 0;
		size_t topology_transitions = 0;
	};

	struct SynthesisReport {
		SynthesisStatus status = SynthesisStatus::NoController;
		std::optional<Candidate> candidate;
		SynthesisStatistics statistics;
	};

	struct SearchControl {
		std::optional<std::chrono::steady_clock::time_point> deadline;
		std::function<void(const Candidate&, const SynthesisStatistics&)> on_best_candidate;

		[[nodiscard]] bool StopRequested() const {
			return deadline.has_value() && std::chrono::steady_clock::now() >= *deadline;
		}
	};

}
