#pragma once

#include <cstddef>
#include <cstdint>
#include <chrono>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "scs/Synthesis/Exact/arena.h"

namespace scs {

	using BudgetState = std::pair<ArenaStateId, uint64_t>;

	struct Controller {
		Arena arena;
		uint64_t optimal_response_cost = 0;
		std::map<BudgetState, ArenaEdgeId> strategy;
		std::optional<SynthesisBackend> backend;
	};

	struct ValidationReport {
		bool valid = false;
		uint64_t worst_response_cost = 0;
		size_t reachable_states = 0;
		std::vector<std::string> diagnostics;
	};

	enum class SynthesisStatus {
		Winning,
		Losing,
		InvalidModel,
		Cancelled,
	};

	struct SynthesisPhaseStatistics {
		std::chrono::nanoseconds arena_construction{};
		std::chrono::nanoseconds qualitative_solving{};
		std::chrono::nanoseconds budget_optimization{};
		std::chrono::nanoseconds extraction{};
		std::chrono::nanoseconds validation{};
	};

	struct SynthesisStatistics {
		ArenaStatistics arena;
		ArenaBounds bounds;
		size_t arena_states = 0;
		size_t arena_edges = 0;
		size_t winning_states = 0;
		size_t winning_controller_states = 0;
		size_t controller_strategy_entries = 0;
		size_t qualitative_iterations = 0;
		size_t budget_tests = 0;
		uint64_t greedy_upper_bound = 0;
		uint64_t theoretical_upper_bound = 0;
		SynthesisPhaseStatistics phases;
	};

	struct SynthesisResult {
		SynthesisStatus status = SynthesisStatus::InvalidModel;
		std::optional<uint64_t> optimal_response_cost;
		std::optional<Controller> controller;
		ValidationReport validation;
		SynthesisStatistics statistics;
		std::vector<std::string> diagnostics;
	};

	SynthesisResult SolveArena(Arena arena,
		std::optional<std::chrono::steady_clock::time_point> deadline = {});
	ValidationReport ValidateController(const SynthesisProblem& problem,
		const SynthesisOptions& options, const Controller& controller);
	ValidationReport ValidateController(const SynthesisProblem& problem,
		const Controller& controller);
	SynthesisResult Synthesise(const SynthesisProblem& problem,
		const SynthesisOptions& options = {});

}
