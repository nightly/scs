#include "scs/ConGolog/CharacteristicGraph/export.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "scs/Common/tex.h"

namespace scs {
namespace {

	template <typename T>
	std::string Render(const T& value) {
		std::ostringstream stream;
		stream << value;
		return stream.str();
	}

}

	std::string CharacteristicGraphToTikz(const CharacteristicGraph& graph) {
		std::vector<CgState> states;
		states.reserve(graph.lts.states().size());
		for (const auto& [state, ignored] : graph.lts.states()) {
			(void)ignored;
			states.push_back(state);
		}
		std::ranges::sort(states, {}, &CgState::n);

		std::ostringstream output;
		output << "\\begin{tikzpicture}[>=stealth]\n";
		for (size_t i = 0; i < states.size(); ++i) {
			const CgState& state = states[i];
			const bool initial = state == graph.lts.initial_state();
			const bool final = graph.lts.final_states().contains(state);
			output << "  \\node[draw,rounded corners";
			if (initial) output << ",very thick";
			if (final) output << ",double";
			output << "] (q" << state.n << ") at (" << (i % 5) * 4 << ',' << -static_cast<int>(i / 5) * 3
				<< ") {\\texttt{" << EscapeTex(Render(state)) << "}};\n";
		}

		struct Edge {
			size_t source = 0;
			size_t target = 0;
			std::string label;
		};
		std::vector<Edge> edges;
		for (const CgState& source : states) {
			for (const auto& transition : graph.lts.at(source).transitions()) {
				edges.push_back({source.n, transition.to().n, Render(transition.label())});
			}
		}
		std::ranges::sort(edges, [](const Edge& lhs, const Edge& rhs) {
			return std::tie(lhs.source, lhs.target, lhs.label) < std::tie(rhs.source, rhs.target, rhs.label);
		});
		for (const Edge& edge : edges) {
			output << "  \\path[->] (q" << edge.source << ") edge";
			if (edge.source == edge.target) output << "[loop above]";
			output << " node[fill=white,font=\\scriptsize] {\\texttt{" << EscapeTex(edge.label)
				<< "}} (q" << edge.target << ");\n";
		}
		output << "\\end{tikzpicture}\n";
		return output.str();
	}

	void ExportGraphTikzToFile(const CharacteristicGraph& graph, const std::filesystem::path& path) {
		std::ofstream stream(path);
		if (!stream) throw std::runtime_error("Unable to open TikZ export file: " + path.string());
		stream << CharacteristicGraphToTikz(graph);
	}

	void ExportGraphTikz(const CharacteristicGraph& graph, std::string_view file_name, std::string_view dir) {
		ExportGraphTikzToFile(graph, std::filesystem::path(dir) / (std::string(file_name) + ".tex"));
	}

}
