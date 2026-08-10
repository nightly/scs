#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

#include "Assembly/assembly.h"
#include "scs/Synthesis/synthesis.h"

namespace {

	scs::WorklistOrder ParseWorklist(const std::string& value) {
		if (value == "breadth-first") return scs::WorklistOrder::BreadthFirst;
		if (value == "lower-cost-first") return scs::WorklistOrder::LowerCostFirst;
		if (value == "greedy") return scs::WorklistOrder::Greedy;
		throw std::invalid_argument("unknown worklist: " + value);
	}

}

int main(int argc, char** argv) {
	try {
		auto options = scs::examples::AssemblySynthesisOptions();
		std::filesystem::path export_prefix;
		for (int i = 1; i < argc; ++i) {
			const std::string argument = argv[i];
			const auto value = [&]() -> std::string {
				if (++i >= argc) throw std::invalid_argument(argument + " requires a value");
				return argv[i];
			};
			if (argument == "--finite") {
				options.backend = scs::FiniteDomainBackend{scs::ObjectSet{
					scs::Object::Identifier("finite-p"), scs::Object::Identifier("finite-q")}};
			} else if (argument == "--worklist") {
				options.backend = scs::FaithfulAbstractionBackend{2, ParseWorklist(value())};
			} else if (argument == "--no-validation") {
				options.validate_controller = false;
			} else if (argument == "--export-prefix") {
				export_prefix = value();
			} else if (argument == "--help" || argument == "-h") {
				std::cout << "Usage: scs_cli [--finite] [--worklist breadth-first|lower-cost-first|greedy] "
					"[--no-validation] [--export-prefix PATH]\n";
				return 0;
			} else {
				throw std::invalid_argument("unknown argument: " + argument);
			}
		}

		auto problem = scs::examples::MakeAssemblyProblem();
		const scs::SynthesisResult result = scs::Synthesise(problem, options);
		if (result.status != scs::SynthesisStatus::Winning) {
			std::cerr << "Synthesis did not produce a controller\n";
			for (const std::string& diagnostic : result.diagnostics) std::cerr << "- " << diagnostic << '\n';
			return 1;
		}
		std::cout << "Winning controller; optimal worst-case response cost K*="
			<< *result.optimal_response_cost << ", states=" << result.statistics.arena_states
			<< ", edges=" << result.statistics.arena_edges << '\n';
		if (!export_prefix.empty()) {
			if (!export_prefix.parent_path().empty()) std::filesystem::create_directories(export_prefix.parent_path());
			scs::ExportControllerGraphViz(*result.controller, export_prefix.string() + ".gv");
			scs::ExportControllerTikz(*result.controller, export_prefix.string() + ".tex");
		}
		return 0;
	} catch (const std::exception& error) {
		std::cerr << "scs_cli: " << error.what() << '\n';
		return 2;
	}
}
