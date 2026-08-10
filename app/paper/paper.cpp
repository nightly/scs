#include "paper.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "Assembly/assembly.h"
#include "scs/Synthesis/Exact/controller.h"
#include "scs/Synthesis/Exact/export.h"

namespace scs::paper {
namespace {

	enum class Suite { All, Smoke, Finite, Worklists, Validation };

	struct Options {
		Suite suite = Suite::All;
		std::optional<std::filesystem::path> output_directory;
		std::optional<std::chrono::milliseconds> timeout;
		bool list = false;
		bool help = false;
	};

	std::string StatusName(SynthesisStatus status) {
		switch (status) {
		case SynthesisStatus::Winning: return "winning";
		case SynthesisStatus::Losing: return "losing";
		case SynthesisStatus::InvalidModel: return "invalid_model";
		case SynthesisStatus::Cancelled: return "cancelled";
		}
		return "invalid_model";
	}

	std::string WorklistName(WorklistOrder order) {
		switch (order) {
		case WorklistOrder::BreadthFirst: return "breadth_first";
		case WorklistOrder::LowerCostFirst: return "lower_cost_first";
		case WorklistOrder::Greedy: return "greedy";
		}
		return "unknown";
	}

	Suite ParseSuite(const std::string& value) {
		if (value == "all") return Suite::All;
		if (value == "smoke") return Suite::Smoke;
		if (value == "finite") return Suite::Finite;
		if (value == "worklists") return Suite::Worklists;
		if (value == "validation") return Suite::Validation;
		throw std::invalid_argument("unknown suite: " + value);
	}

	Options ParseOptions(int argc, char** argv) {
		Options options;
		for (int i = 1; i < argc; ++i) {
			const std::string argument = argv[i];
			const auto value = [&]() -> std::string {
				if (++i >= argc) throw std::invalid_argument(argument + " requires a value");
				return argv[i];
			};
			if (argument == "--suite") options.suite = ParseSuite(value());
			else if (argument == "--output-dir") options.output_directory = value();
			else if (argument == "--timeout-ms") {
				const auto milliseconds = std::stoll(value());
				if (milliseconds <= 0) throw std::invalid_argument("--timeout-ms must be positive");
				options.timeout = std::chrono::milliseconds(milliseconds);
			} else if (argument == "--list") options.list = true;
			else if (argument == "--help" || argument == "-h") options.help = true;
			else throw std::invalid_argument("unknown argument: " + argument);
		}
		return options;
	}

	void Usage() {
		std::cout << "Usage: scs_paper [--suite all|smoke|finite|worklists|validation] "
			"[--output-dir PATH] [--timeout-ms N] [--list]\n";
	}

	void List(Suite suite) {
		const auto includes = [suite](Suite requested) {
			return suite == Suite::All || suite == requested || suite == Suite::Smoke;
		};
		if (includes(Suite::Validation)) std::cout << "faithful-breadth-first (independently validated)\n";
		if (suite == Suite::All || suite == Suite::Finite) std::cout << "finite-explicit and faithful representation comparison\n";
		if (suite == Suite::All || suite == Suite::Worklists) std::cout << "breadth-first, lower-cost-first, and greedy worklists\n";
	}

	std::filesystem::path OutputDirectory(const Options& options) {
		std::filesystem::path directory = options.output_directory.value_or(
			std::filesystem::path("exports") / "paper-exact");
		if (std::filesystem::exists(directory)
			&& std::filesystem::directory_iterator(directory) != std::filesystem::directory_iterator{}) {
			throw std::runtime_error("output directory is not empty: " + directory.string());
		}
		std::filesystem::create_directories(directory);
		return std::filesystem::absolute(directory);
	}

	double Milliseconds(std::chrono::nanoseconds duration) {
		return std::chrono::duration<double, std::milli>(duration).count();
	}

	void ExerciseFreshController(const SynthesisProblem& problem, const Controller& controller) {
		ControllerSession session{problem, controller, SequentialFreshIdentifiers("paper-controller-")};
		for (size_t cycle = 0; cycle < 3; ++cycle) {
			const Object p = Object::Identifier("paper-p-" + std::to_string(cycle));
			const Object q = Object::Identifier("paper-q-" + std::to_string(cycle));
			RecipeEdgeChoice first{CompoundAction{Action{"load", {p, Object::Rigid("brass")}}}};
			first.bindings.Set(Variable{"p"}, p);
			first.bindings.Set(Variable{"q"}, q);
			const ControllerResponse first_response = session.Respond(first);
			if (first_response.cost > controller.optimal_response_cost
				|| RenameableActiveDomainSize(session.concrete_state()) > 2) {
				throw std::runtime_error("fresh renamed controller execution violated its bound");
			}
			for (const CompoundAction& request : {
				CompoundAction{Action{"load", {q, Object::Rigid("tube")}}},
				CompoundAction{Action{"drill", {p, Object::Rigid("bit5")}}},
				CompoundAction{Action{"join", {q, p}}},
				CompoundAction{Action{"store", {p, Object::Rigid("ok")}}}}) {
				const ControllerResponse response = session.Respond(request);
				if (response.cost > controller.optimal_response_cost
					|| RenameableActiveDomainSize(session.concrete_state()) > 2) {
					throw std::runtime_error("fresh renamed controller execution violated its bound");
				}
			}
		}
		session.Stop();
	}

	struct Case {
		std::string name;
		SynthesisOptions options;
		bool exercise = false;
	};

	std::vector<Case> Cases(const Options& options) {
		std::vector<Case> cases;
		const auto faithful = [&](WorklistOrder order, bool validate = true) {
			auto synthesis = examples::AssemblySynthesisOptions(order);
			synthesis.validate_controller = validate;
			return synthesis;
		};
		if (options.suite == Suite::Smoke || options.suite == Suite::Validation) {
			cases.push_back({"faithful-breadth-first", faithful(WorklistOrder::BreadthFirst), true});
		} else if (options.suite == Suite::Finite) {
			SynthesisOptions finite;
			finite.backend = FiniteDomainBackend{ObjectSet{
				Object::Identifier("finite-p"), Object::Identifier("finite-q")}};
			cases.push_back({"finite-explicit", finite, false});
			cases.push_back({"faithful-breadth-first", faithful(WorklistOrder::BreadthFirst), true});
		} else if (options.suite == Suite::Worklists) {
			for (const WorklistOrder order : {WorklistOrder::BreadthFirst,
				WorklistOrder::LowerCostFirst, WorklistOrder::Greedy}) {
				cases.push_back({"faithful-" + WorklistName(order), faithful(order), order == WorklistOrder::BreadthFirst});
			}
		} else {
			SynthesisOptions finite;
			finite.backend = FiniteDomainBackend{ObjectSet{
				Object::Identifier("finite-p"), Object::Identifier("finite-q")}};
			cases.push_back({"finite-explicit", finite, false});
			for (const WorklistOrder order : {WorklistOrder::BreadthFirst,
				WorklistOrder::LowerCostFirst, WorklistOrder::Greedy}) {
				cases.push_back({"faithful-" + WorklistName(order), faithful(order), order == WorklistOrder::BreadthFirst});
			}
		}
		return cases;
	}

}

	int Run(int argc, char** argv) {
		try {
			const Options options = ParseOptions(argc, argv);
			if (options.help) {
				Usage();
				return 0;
			}
			if (options.list) {
				List(options.suite);
				return 0;
			}
			const std::filesystem::path directory = OutputDirectory(options);
			std::ofstream metrics(directory / "metrics.tsv");
			metrics << "case\tstatus\toptimal_response_cost\tvalidated\tactive_domain\trecipe_live\tfacility_live\t"
				"recipe_action_arity\tsupport\tfresh_edge\tpool\tarena_states\tarena_edges\tgroundings\tprogressions\t"
				"isomorphism_checks\tcallback_contract_samples\tqualitative_iterations\tbudget_tests\tgreedy_upper_bound\tkmax\t"
				"arena_ms\tqualitative_ms\tbudget_ms\textraction_ms\tvalidation_ms\n";
			bool success = true;
			for (Case item : Cases(options)) {
				if (options.timeout) {
					item.options.deadline = std::chrono::steady_clock::now() + *options.timeout;
				}
				auto problem = examples::MakeAssemblyProblem();
				const SynthesisResult result = Synthesise(problem, item.options);
				const auto& stats = result.statistics;
				metrics << item.name << '\t' << StatusName(result.status) << '\t'
					<< result.optimal_response_cost.value_or(0) << '\t' << result.validation.valid << '\t'
					<< stats.bounds.active_domain << '\t' << stats.bounds.recipe_live << '\t'
					<< stats.bounds.facility_live << '\t' << stats.bounds.recipe_action_arity << '\t'
					<< stats.bounds.support << '\t' << stats.bounds.fresh_edge << '\t' << stats.bounds.pool << '\t'
					<< stats.arena_states << '\t' << stats.arena_edges << '\t'
					<< stats.arena.generated_substitutions << '\t' << stats.arena.progression_calls << '\t'
					<< stats.arena.isomorphism_checks << '\t' << stats.arena.callback_contract_samples << '\t'
					<< stats.qualitative_iterations << '\t'
					<< stats.budget_tests << '\t' << stats.greedy_upper_bound << '\t'
					<< stats.theoretical_upper_bound << '\t' << std::setprecision(10)
					<< Milliseconds(stats.phases.arena_construction) << '\t'
					<< Milliseconds(stats.phases.qualitative_solving) << '\t'
					<< Milliseconds(stats.phases.budget_optimization) << '\t'
					<< Milliseconds(stats.phases.extraction) << '\t'
					<< Milliseconds(stats.phases.validation) << '\n';
				metrics.flush();
				std::cout << item.name << ": " << StatusName(result.status);
				if (result.optimal_response_cost) std::cout << ", K*=" << *result.optimal_response_cost;
				std::cout << '\n';
				if (result.status != SynthesisStatus::Winning || result.optimal_response_cost != 10) success = false;
				if (result.controller) {
					ExportControllerGraphViz(*result.controller, directory / (item.name + "-controller.gv"));
					ExportControllerTikz(*result.controller, directory / (item.name + "-controller.tex"));
					if (item.exercise) ExerciseFreshController(problem, *result.controller);
				}
			}
			return success ? 0 : 1;
		} catch (const std::exception& error) {
			std::cerr << "scs_paper: " << error.what() << '\n';
			return 2;
		}
	}

}
