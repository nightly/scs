#include "scs/Synthesis/Exact/solver.h"

#include <algorithm>
#include <deque>
#include <functional>
#include <limits>
#include <set>
#include <stdexcept>

namespace scs {
namespace {

	struct GameEdge {
		size_t target = 0;
		ArenaEdgeId arena_edge = 0;
	};

	struct FiniteGame {
		std::vector<ArenaOwner> owner;
		std::vector<std::vector<GameEdge>> outgoing;
		std::vector<BudgetState> states;
		size_t initial = 0;
		size_t goal = 0;
		size_t lose = 0;
	};

	struct WinningRegion {
		std::vector<bool> contains;
		std::vector<size_t> rank;
		size_t iterations = 0;
		bool cancelled = false;
	};

	FiniteGame BaseGame(const Arena& arena) {
		FiniteGame game;
		game.owner.reserve(arena.states.size());
		game.outgoing.resize(arena.states.size());
		game.states.reserve(arena.states.size());
		for (ArenaStateId id = 0; id < arena.states.size(); ++id) {
			game.owner.push_back(arena.states[id].owner);
			game.states.emplace_back(id, 0);
			for (const auto edge_id : arena.outgoing[id]) {
				game.outgoing[id].push_back({arena.edges[edge_id].target, edge_id});
			}
		}
		game.initial = arena.initial;
		game.goal = arena.goal;
		game.lose = arena.lose;
		return game;
	}

	FiniteGame BudgetGame(const Arena& arena, uint64_t budget) {
		FiniteGame game;
		std::map<BudgetState, size_t> index;
		std::deque<size_t> worklist;
		auto add = [&](const BudgetState& state) {
			const auto [found, inserted] = index.try_emplace(state, game.states.size());
			if (inserted) {
				game.states.emplace_back(state);
				game.owner.emplace_back(arena.states.at(state.first).owner);
				game.outgoing.emplace_back();
				worklist.push_back(found->second);
			}
			return found->second;
		};
		game.initial = add({arena.initial, 0});
		game.goal = add({arena.goal, 0});
		game.lose = add({arena.lose, 0});
		while (!worklist.empty()) {
			const size_t node = worklist.front();
			worklist.pop_front();
			const auto [state, accumulated] = game.states[node];
			for (const auto edge_id : arena.outgoing[state]) {
				const auto& edge = arena.edges[edge_id];
				BudgetState target{arena.lose, 0};
				if (arena.states[state].owner != ArenaOwner::Controller
					|| (accumulated <= budget && edge.cost <= budget - accumulated)) {
					const uint64_t next_cost = arena.states[state].owner == ArenaOwner::Controller
						&& arena.states[edge.target].owner == ArenaOwner::Controller
						? accumulated + edge.cost : 0;
					target = {edge.target, next_cost};
				}
				const size_t target_node = add(target);
				game.outgoing[node].push_back({target_node, edge_id});
			}
		}
		return game;
	}

	WinningRegion ComputeWinning(const FiniteGame& game,
		std::optional<std::chrono::steady_clock::time_point> deadline = {}) {
		WinningRegion result;
		std::vector<bool> outer(game.states.size(), true);
		while (true) {
			if (deadline && std::chrono::steady_clock::now() >= *deadline) {
				result.cancelled = true;
				return result;
			}
			++result.iterations;
			std::vector<bool> inner(game.states.size(), false);
			std::vector<size_t> rank(game.states.size(), std::numeric_limits<size_t>::max());
			inner[game.goal] = true;
			rank[game.goal] = 0;
			size_t current_rank = 0;
			while (true) {
				if (deadline && std::chrono::steady_clock::now() >= *deadline) {
					result.cancelled = true;
					return result;
				}
				std::vector<bool> next = inner;
				++current_rank;
				for (size_t state = 0; state < game.states.size(); ++state) {
					if (inner[state]) continue;
					if (game.owner[state] == ArenaOwner::Environment) {
						const bool all = !game.outgoing[state].empty()
							&& std::ranges::all_of(game.outgoing[state],
								[&](const GameEdge& edge) { return outer[edge.target]; });
						if (all) {
							next[state] = true;
							rank[state] = current_rank;
						}
					} else if (game.owner[state] == ArenaOwner::Controller) {
						const bool some = std::ranges::any_of(game.outgoing[state],
							[&](const GameEdge& edge) { return inner[edge.target]; });
						if (some) {
							next[state] = true;
							rank[state] = current_rank;
						}
					}
				}
				if (next == inner) break;
				inner = std::move(next);
			}
			if (inner == outer) {
				result.contains = std::move(inner);
				result.rank = std::move(rank);
				return result;
			}
			outer = std::move(inner);
		}
	}

	std::map<size_t, ArenaEdgeId> ExtractRankStrategy(const FiniteGame& game,
		const Arena& arena, const WinningRegion& winning) {
		std::map<size_t, ArenaEdgeId> strategy;
		for (size_t state = 0; state < game.states.size(); ++state) {
			if (!winning.contains[state] || game.owner[state] != ArenaOwner::Controller) continue;
			std::optional<GameEdge> best;
			for (const auto& edge : game.outgoing[state]) {
				if (!winning.contains[edge.target]
					|| winning.rank[edge.target] >= winning.rank[state]) continue;
				if (!best || arena.edges[edge.arena_edge].cost < arena.edges[best->arena_edge].cost
					|| (arena.edges[edge.arena_edge].cost == arena.edges[best->arena_edge].cost
						&& edge.arena_edge < best->arena_edge)) {
					best = edge;
				}
			}
			if (!best) throw std::logic_error("Winning Controller state has no rank-decreasing edge");
			strategy.emplace(state, best->arena_edge);
		}
		return strategy;
	}

	uint64_t GreedyUpperBound(const FiniteGame& game, const Arena& arena,
		const WinningRegion& winning, const std::map<size_t, ArenaEdgeId>& strategy) {
		std::vector<std::optional<uint64_t>> memo(game.states.size());
		const auto response = [&](this const auto& self, size_t node) -> uint64_t {
			if (game.owner[node] != ArenaOwner::Controller) return 0;
			if (memo[node]) return *memo[node];
			const auto found = strategy.find(node);
			if (found == strategy.end()) throw std::logic_error("Incomplete greedy strategy");
			const auto edge_it = std::ranges::find(game.outgoing[node], found->second,
				&GameEdge::arena_edge);
			if (edge_it == game.outgoing[node].end()) throw std::logic_error("Strategy edge is not outgoing");
			const uint64_t suffix = self(edge_it->target);
			if (arena.edges[found->second].cost > std::numeric_limits<uint64_t>::max() - suffix) {
				throw std::overflow_error("Response cost overflow");
			}
			return *(memo[node] = arena.edges[found->second].cost + suffix);
		};

		uint64_t result = 0;
		for (size_t node = 0; node < game.states.size(); ++node) {
			if (winning.contains[node] && game.owner[node] == ArenaOwner::Controller) {
				result = std::max(result, response(node));
			}
		}
		return result;
	}

	std::optional<size_t> FindEquivalentState(const Arena& arena, const ArenaState& state) {
		const auto found = std::ranges::find(arena.states, state);
		if (found == arena.states.end()) return std::nullopt;
		return static_cast<size_t>(std::distance(arena.states.begin(), found));
	}

	bool SameLabel(const ArenaLabel& lhs, const ArenaLabel& rhs) {
		return lhs == rhs;
	}

}

	SynthesisResult SolveArena(Arena arena,
		std::optional<std::chrono::steady_clock::time_point> deadline) {
		SynthesisResult result;
		result.statistics.arena = arena.statistics;
		result.statistics.bounds = arena.bounds;
		result.statistics.arena_states = arena.states.size();
		result.statistics.arena_edges = arena.edges.size();
		const auto qualitative_started = std::chrono::steady_clock::now();
		const FiniteGame base = BaseGame(arena);
		const WinningRegion qualitative = ComputeWinning(base, deadline);
		result.statistics.phases.qualitative_solving =
			std::chrono::steady_clock::now() - qualitative_started;
		result.statistics.qualitative_iterations = qualitative.iterations;
		if (qualitative.cancelled) {
			result.status = SynthesisStatus::Cancelled;
			return result;
		}
		if (!qualitative.contains[base.initial]) {
			result.status = SynthesisStatus::Losing;
			return result;
		}
		result.statistics.winning_states = std::ranges::count(qualitative.contains, true);

		const auto greedy_strategy = ExtractRankStrategy(base, arena, qualitative);
		const uint64_t greedy_upper = GreedyUpperBound(base, arena, qualitative, greedy_strategy);
		result.statistics.greedy_upper_bound = greedy_upper;
		uint64_t maximum_edge_cost = 0;
		size_t controller_states = 0;
		for (size_t state = 0; state < arena.states.size(); ++state) {
			if (qualitative.contains[state] && arena.states[state].owner == ArenaOwner::Controller) {
				++controller_states;
			}
		}
		result.statistics.winning_controller_states = controller_states;
		for (const auto& edge : arena.edges) maximum_edge_cost = std::max(maximum_edge_cost, edge.cost);
		const uint64_t theoretical = controller_states != 0
			&& maximum_edge_cost > std::numeric_limits<uint64_t>::max() / controller_states
			? std::numeric_limits<uint64_t>::max()
			: maximum_edge_cost * controller_states;
		result.statistics.theoretical_upper_bound = theoretical;
		uint64_t low = 0;
		uint64_t high = std::min(greedy_upper, theoretical);
		const auto budget_started = std::chrono::steady_clock::now();
		while (low < high) {
			if (deadline && std::chrono::steady_clock::now() >= *deadline) {
				result.status = SynthesisStatus::Cancelled;
				return result;
			}
			const uint64_t middle = low + (high - low) / 2;
			++result.statistics.budget_tests;
			const FiniteGame budget_game = BudgetGame(arena, middle);
			const auto winning = ComputeWinning(budget_game, deadline);
			if (winning.cancelled) {
				result.status = SynthesisStatus::Cancelled;
				return result;
			}
			if (winning.contains[budget_game.initial]) high = middle;
			else low = middle + 1;
		}
		++result.statistics.budget_tests;
		FiniteGame optimal_game = BudgetGame(arena, low);
		WinningRegion optimal_winning = ComputeWinning(optimal_game, deadline);
		result.statistics.phases.budget_optimization =
			std::chrono::steady_clock::now() - budget_started;
		if (optimal_winning.cancelled) {
			result.status = SynthesisStatus::Cancelled;
			return result;
		}
		if (!optimal_winning.contains[optimal_game.initial]) {
			result.status = SynthesisStatus::InvalidModel;
			result.diagnostics.emplace_back("Greedy upper bound was not budget-feasible");
			return result;
		}
		const auto extraction_started = std::chrono::steady_clock::now();
		const auto strategy = ExtractRankStrategy(optimal_game, arena, optimal_winning);
		Controller controller;
		controller.arena = std::move(arena);
		controller.optimal_response_cost = low;
		for (const auto& [node, edge] : strategy) {
			controller.strategy.emplace(optimal_game.states[node], edge);
		}
		result.statistics.controller_strategy_entries = controller.strategy.size();
		for (ArenaStateId state = 0; state < controller.arena.states.size(); ++state) {
			if (controller.arena.states[state].owner != ArenaOwner::Controller) continue;
			auto& outgoing = controller.arena.outgoing[state];
			std::erase_if(outgoing, [&](ArenaEdgeId edge) {
				return std::ranges::none_of(controller.strategy, [&](const auto& selected) {
					return selected.first.first == state && selected.second == edge;
				});
			});
		}
		result.statistics.phases.extraction =
			std::chrono::steady_clock::now() - extraction_started;
		result.status = SynthesisStatus::Winning;
		result.optimal_response_cost = low;
		result.controller = std::move(controller);
		return result;
	}

	ValidationReport ValidateController(const SynthesisProblem& problem,
		const SynthesisOptions& options, const Controller& controller) {
		ValidationReport report;
		SynthesisOptions rebuild_options = options;
		rebuild_options.validate_controller = false;
		const ArenaBuildResult rebuilt = BuildArena(problem, rebuild_options);
		if (rebuilt.status != ArenaBuildStatus::Complete) {
			report.diagnostics.emplace_back("Independent arena regeneration failed");
			report.diagnostics.insert(report.diagnostics.end(), rebuilt.diagnostics.begin(), rebuilt.diagnostics.end());
			return report;
		}

		for (const auto& edge : controller.arena.edges) {
			if (!IsBijectionWitness(edge.witness)) {
				report.diagnostics.emplace_back("Controller contains a non-bijective edge witness");
				return report;
			}
		}

		using Visit = std::pair<ArenaStateId, uint64_t>;
		std::deque<Visit> worklist{{controller.arena.initial, 0}};
		std::set<Visit> visited;
		while (!worklist.empty()) {
			const auto [state, accumulated] = worklist.front();
			worklist.pop_front();
			if (!visited.emplace(state, accumulated).second) continue;
			if (state == controller.arena.lose
				|| controller.arena.states[state].owner == ArenaOwner::Lose) {
				report.diagnostics.emplace_back("Controller reaches the losing sink");
				return report;
			}
			if (controller.arena.states[state].owner == ArenaOwner::Goal) continue;

			const auto fresh_source = FindEquivalentState(rebuilt.arena, controller.arena.states[state]);
			if (!fresh_source) {
				report.diagnostics.emplace_back("Controller state is absent from the regenerated arena");
				return report;
			}
			if (controller.arena.states[state].owner == ArenaOwner::Environment) {
				for (const auto fresh_edge_id : rebuilt.arena.outgoing[*fresh_source]) {
					const auto& fresh_edge = rebuilt.arena.edges[fresh_edge_id];
					const auto target = FindEquivalentState(controller.arena,
						rebuilt.arena.states[fresh_edge.target]);
					if (!target) {
						report.diagnostics.emplace_back("Controller omits an Environment successor state");
						return report;
					}
					const bool covered = std::ranges::any_of(controller.arena.outgoing[state],
						[&](ArenaEdgeId edge_id) {
							const auto& edge = controller.arena.edges[edge_id];
							return edge.target == *target && SameLabel(edge.label, fresh_edge.label)
								&& edge.cost == fresh_edge.cost && edge.witness == fresh_edge.witness;
						});
					if (!covered) {
						report.diagnostics.emplace_back("Controller omits an enabled Environment edge");
						return report;
					}
				}
				for (const auto edge_id : controller.arena.outgoing[state]) {
					worklist.emplace_back(controller.arena.edges[edge_id].target, 0);
				}
			} else {
				const auto selected = controller.strategy.find({state, accumulated});
				if (selected == controller.strategy.end()) {
					report.diagnostics.emplace_back("Controller strategy is undefined at a reachable state");
					return report;
				}
				const auto& edge = controller.arena.edges.at(selected->second);
				if (edge.source != state || !std::ranges::contains(controller.arena.outgoing[state], selected->second)) {
					report.diagnostics.emplace_back("Controller selects an edge with the wrong source");
					return report;
				}
				const auto fresh_target = FindEquivalentState(rebuilt.arena,
					controller.arena.states[edge.target]);
				const bool executable = fresh_target && std::ranges::any_of(
					rebuilt.arena.outgoing[*fresh_source], [&](ArenaEdgeId fresh_edge_id) {
						const auto& fresh = rebuilt.arena.edges[fresh_edge_id];
						return fresh.target == *fresh_target && SameLabel(fresh.label, edge.label)
							&& fresh.cost == edge.cost && fresh.witness == edge.witness;
					});
				if (!executable) {
					report.diagnostics.emplace_back("Selected Controller edge is not semantically executable");
					return report;
				}
				if (accumulated > controller.optimal_response_cost
					|| edge.cost > controller.optimal_response_cost - accumulated) {
					report.diagnostics.emplace_back("Controller exceeds its reported response bound");
					return report;
				}
				const uint64_t total = accumulated + edge.cost;
				report.worst_response_cost = std::max(report.worst_response_cost, total);
				const uint64_t next = controller.arena.states[edge.target].owner == ArenaOwner::Controller
					? total : 0;
				worklist.emplace_back(edge.target, next);
			}
		}
		std::map<Visit, unsigned char> response_colours;
		const auto response_terminates = [&](this const auto& self, const Visit& visit) -> bool {
			if (!visited.contains(visit)
				|| controller.arena.states[visit.first].owner != ArenaOwner::Controller) return true;
			auto& colour = response_colours[visit];
			if (colour == 1) return false;
			if (colour == 2) return true;
			colour = 1;
			const auto selected = controller.strategy.find(visit);
			if (selected == controller.strategy.end()) return false;
			const auto& edge = controller.arena.edges.at(selected->second);
			if (controller.arena.states[edge.target].owner == ArenaOwner::Controller) {
				if (edge.cost > std::numeric_limits<uint64_t>::max() - visit.second
					|| !self(Visit{edge.target, visit.second + edge.cost})) return false;
			}
			colour = 2;
			return true;
		};
		for (const Visit& visit : visited) {
			if (!response_terminates(visit)) {
				report.diagnostics.emplace_back("Controller contains a nonterminating response cycle");
				return report;
			}
		}
		report.reachable_states = visited.size();
		if (report.worst_response_cost != controller.optimal_response_cost) {
			report.diagnostics.emplace_back("Recomputed response cost differs from the reported optimum");
			return report;
		}
		report.valid = true;
		return report;
	}

	ValidationReport ValidateController(const SynthesisProblem& problem,
		const Controller& controller) {
		if (!controller.backend) {
			ValidationReport report;
			report.diagnostics.emplace_back(
				"Controller does not record the synthesis backend required for independent validation");
			return report;
		}
		SynthesisOptions options;
		options.backend = *controller.backend;
		options.validate_controller = false;
		return ValidateController(problem, options, controller);
	}

	SynthesisResult Synthesise(const SynthesisProblem& problem, const SynthesisOptions& options) {
		const auto arena_started = std::chrono::steady_clock::now();
		ArenaBuildResult build = BuildArena(problem, options);
		const auto arena_duration = std::chrono::steady_clock::now() - arena_started;
		if (build.status == ArenaBuildStatus::InvalidModel) {
			SynthesisResult result;
			result.status = SynthesisStatus::InvalidModel;
			result.statistics.phases.arena_construction = arena_duration;
			result.diagnostics = std::move(build.diagnostics);
			return result;
		}
		if (build.status == ArenaBuildStatus::Cancelled) {
			SynthesisResult result;
			result.status = SynthesisStatus::Cancelled;
			result.statistics.phases.arena_construction = arena_duration;
			return result;
		}
		SynthesisResult result;
		try {
			result = SolveArena(std::move(build.arena), options.deadline);
		} catch (const std::exception& error) {
			result.status = SynthesisStatus::InvalidModel;
			result.diagnostics.emplace_back(error.what());
		}
		result.statistics.phases.arena_construction = arena_duration;
		if (result.controller) result.controller->backend = options.backend;
		if (result.status == SynthesisStatus::Winning && options.validate_controller) {
			const auto validation_started = std::chrono::steady_clock::now();
			result.validation = ValidateController(problem, *result.controller);
			result.statistics.phases.validation =
				std::chrono::steady_clock::now() - validation_started;
			if (options.deadline && std::chrono::steady_clock::now() >= *options.deadline) {
				result.status = SynthesisStatus::Cancelled;
			} else if (!result.validation.valid) {
				result.status = SynthesisStatus::InvalidModel;
				result.diagnostics.insert(result.diagnostics.end(),
					result.validation.diagnostics.begin(), result.validation.diagnostics.end());
			}
		}
		return result;
	}

}
