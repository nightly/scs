#include "scs/Synthesis/Exact/export.h"

#include <deque>
#include <fstream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <tuple>

#include "scs/Common/tex.h"

namespace scs {
namespace {

	struct ControllerEdge {
		BudgetState source;
		BudgetState target;
		ArenaEdgeId arena_edge = 0;
	};

	struct ControllerSubgraph {
		std::set<BudgetState> states;
		std::vector<ControllerEdge> edges;
	};

	std::string RenderLabel(const ArenaEdge& edge) {
		std::ostringstream output;
		std::visit([&](const auto& label) {
			using T = std::decay_t<decltype(label)>;
			if constexpr (std::is_same_v<T, AuxiliaryLabel>) {
				switch (label) {
				case AuxiliaryLabel::Stop: output << "stop"; break;
				case AuxiliaryLabel::Dead: output << "dead"; break;
				case AuxiliaryLabel::Sink: output << "sink"; break;
				}
			} else {
				output << label;
			}
		}, edge.label);
		if (edge.cost > 0) output << " / " << edge.cost;
		return output.str();
	}

	std::string OwnerName(ArenaOwner owner) {
		switch (owner) {
		case ArenaOwner::Environment: return "E";
		case ArenaOwner::Controller: return "C";
		case ArenaOwner::Goal: return "goal";
		case ArenaOwner::Lose: return "lose";
		}
		return "?";
	}

	std::string StateLabel(const Controller& controller, const BudgetState& state) {
		std::ostringstream output;
		output << OwnerName(controller.arena.states[state.first].owner) << state.first;
		if (controller.arena.states[state.first].owner == ArenaOwner::Controller) output << " [" << state.second << ']';
		return output.str();
	}

	ControllerSubgraph BuildSubgraph(const Controller& controller) {
		ControllerSubgraph result;
		std::deque<BudgetState> worklist{{controller.arena.initial, 0}};
		while (!worklist.empty()) {
			const BudgetState source = worklist.front();
			worklist.pop_front();
			if (!result.states.insert(source).second) continue;
			const ArenaState& arena_state = controller.arena.states.at(source.first);
			std::vector<ArenaEdgeId> selected;
			if (arena_state.owner == ArenaOwner::Controller) {
				const auto strategy = controller.strategy.find(source);
				if (strategy != controller.strategy.end()) selected.push_back(strategy->second);
			} else {
				selected = controller.arena.outgoing.at(source.first);
			}
			std::ranges::sort(selected);
			for (const ArenaEdgeId edge_id : selected) {
				const ArenaEdge& edge = controller.arena.edges.at(edge_id);
				const bool continues = controller.arena.states.at(edge.target).owner == ArenaOwner::Controller;
				const uint64_t budget = arena_state.owner == ArenaOwner::Controller && continues
					? source.second + edge.cost : 0;
				const BudgetState target{edge.target, budget};
				result.edges.push_back({source, target, edge_id});
				if (!result.states.contains(target)) worklist.push_back(target);
			}
		}
		std::ranges::sort(result.edges, [&](const ControllerEdge& lhs, const ControllerEdge& rhs) {
			return std::tie(lhs.source, lhs.target, lhs.arena_edge) <
				std::tie(rhs.source, rhs.target, rhs.arena_edge);
		});
		return result;
	}

	std::string DotEscape(std::string_view value) {
		std::string result;
		for (const char character : value) {
			if (character == '\\' || character == '"') result += '\\';
			result += character == '\n' ? ' ' : character;
		}
		return result;
	}

	std::string NodeId(const BudgetState& state) {
		return "s" + std::to_string(state.first) + "b" + std::to_string(state.second);
	}

}

	std::string ControllerToGraphViz(const Controller& controller) {
		const ControllerSubgraph graph = BuildSubgraph(controller);
		std::ostringstream output;
		output << "digraph Controller {\n  rankdir=LR;\n";
		for (const BudgetState& state : graph.states) {
			const ArenaOwner owner = controller.arena.states[state.first].owner;
			output << "  " << NodeId(state) << " [label=\"" << DotEscape(StateLabel(controller, state)) << "\"";
			if (state.first == controller.arena.initial) output << ", penwidth=2";
			if (owner == ArenaOwner::Goal) output << ", shape=doublecircle";
			else if (owner == ArenaOwner::Lose) output << ", shape=octagon";
			else if (owner == ArenaOwner::Controller) output << ", shape=box";
			output << "];\n";
		}
		for (const ControllerEdge& selected : graph.edges) {
			const ArenaEdge& edge = controller.arena.edges[selected.arena_edge];
			output << "  " << NodeId(selected.source) << " -> " << NodeId(selected.target)
				<< " [label=\"" << DotEscape(RenderLabel(edge)) << "\"];\n";
		}
		output << "}\n";
		return output.str();
	}

	std::string ControllerToTikz(const Controller& controller) {
		const ControllerSubgraph graph = BuildSubgraph(controller);
		std::vector<BudgetState> states(graph.states.begin(), graph.states.end());
		std::ostringstream output;
		output << "\\begin{tikzpicture}[>=stealth]\n";
		for (size_t i = 0; i < states.size(); ++i) {
			const BudgetState state = states[i];
			const ArenaOwner owner = controller.arena.states[state.first].owner;
			output << "  \\node[draw," << (owner == ArenaOwner::Controller ? "rectangle" : "rounded corners");
			if (state.first == controller.arena.initial) output << ",very thick";
			if (owner == ArenaOwner::Goal) output << ",double";
			output << "] (" << NodeId(state) << ") at (" << (i % 5) * 4 << ',' << -static_cast<int>(i / 5) * 3
				<< ") {\\texttt{" << EscapeTex(StateLabel(controller, state)) << "}};\n";
		}
		for (const ControllerEdge& selected : graph.edges) {
			const ArenaEdge& edge = controller.arena.edges[selected.arena_edge];
			output << "  \\path[->] (" << NodeId(selected.source) << ") edge";
			if (selected.source == selected.target) output << "[loop above]";
			output << " node[fill=white,font=\\scriptsize] {\\texttt{" << EscapeTex(RenderLabel(edge))
				<< "}} (" << NodeId(selected.target) << ");\n";
		}
		output << "\\end{tikzpicture}\n";
		return output.str();
	}

	void ExportControllerGraphViz(const Controller& controller, const std::filesystem::path& path) {
		std::ofstream stream(path);
		if (!stream) throw std::runtime_error("Unable to open GraphViz export file: " + path.string());
		stream << ControllerToGraphViz(controller);
	}

	void ExportControllerTikz(const Controller& controller, const std::filesystem::path& path) {
		std::ofstream stream(path);
		if (!stream) throw std::runtime_error("Unable to open TikZ export file: " + path.string());
		stream << ControllerToTikz(controller);
	}

}
