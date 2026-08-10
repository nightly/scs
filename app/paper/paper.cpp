#include "paper.h"

#include <algorithm>
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

	enum class Suite { All, Smoke, Finite, Worklists, Validation, Scaling };

	struct Options {
		Suite suite = Suite::All;
		std::optional<std::filesystem::path> output_directory;
		std::optional<std::chrono::milliseconds> timeout;
		std::optional<std::string> case_name;
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
		case WorklistOrder::BreadthFirst: return "breadth-first";
		case WorklistOrder::LowerCostFirst: return "lower-cost-first";
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
		if (value == "scaling") return Suite::Scaling;
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
			else if (argument == "--case") options.case_name = value();
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
		std::cout << "Usage: scs_paper [--suite all|smoke|finite|worklists|validation|scaling] "
			"[--case NAME] [--output-dir PATH] [--timeout-ms N] [--list]\n";
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

	struct ExerciseStatistics {
		size_t cycles = 0;
		size_t requests = 0;
		uint64_t maximum_response_cost = 0;
		size_t maximum_active_domain = 0;
	};

	ExerciseStatistics ExerciseFreshController(
		const SynthesisProblem& problem, const Controller& controller) {
		ExerciseStatistics statistics;
		ControllerSession session{problem, controller, SequentialFreshIdentifiers("paper-controller-")};
		const auto check = [&](const ControllerResponse& response) {
			++statistics.requests;
			statistics.maximum_response_cost = std::max(statistics.maximum_response_cost, response.cost);
			statistics.maximum_active_domain = std::max(
				statistics.maximum_active_domain, RenameableActiveDomainSize(session.concrete_state()));
			if (response.cost > controller.optimal_response_cost
				|| RenameableActiveDomainSize(session.concrete_state()) > 2) {
				throw std::runtime_error("fresh renamed controller execution violated its bound");
			}
		};
		for (size_t cycle = 0; cycle < 3; ++cycle) {
			const Object p = Object::Identifier("paper-p-" + std::to_string(cycle));
			const Object q = Object::Identifier("paper-q-" + std::to_string(cycle));
			RecipeEdgeChoice first{CompoundAction{Action{"load", {p, Object::Rigid("brass")}}}};
			first.bindings.Set(Variable{"p"}, p);
			first.bindings.Set(Variable{"q"}, q);
			check(session.Respond(first));
			for (const CompoundAction& request : {
				CompoundAction{Action{"load", {q, Object::Rigid("tube")}}},
				CompoundAction{Action{"drill", {p, Object::Rigid("bit5")}}},
				CompoundAction{Action{"join", {q, p}}},
				CompoundAction{Action{"store", {p, Object::Rigid("ok")}}}}) {
				check(session.Respond(request));
			}
			++statistics.cycles;
		}
		session.Stop();
		return statistics;
	}

	struct Case {
		std::string name;
		SynthesisOptions options;
		bool exercise = false;
	};

	SynthesisOptions FiniteOptions(size_t identifiers) {
		ObjectSet carrier;
		for (size_t i = 0; i < identifiers; ++i) {
			carrier.emplace(Object::Identifier("finite-" + std::to_string(i)));
		}
		SynthesisOptions options;
		options.backend = FiniteDomainBackend{std::move(carrier)};
		return options;
	}

	std::vector<Case> AllCases() {
		std::vector<Case> cases;
		const auto faithful = [&](WorklistOrder order, bool validate = true) {
			auto synthesis = examples::AssemblySynthesisOptions(order);
			synthesis.validate_controller = validate;
			return synthesis;
		};
		cases.push_back({"finite-explicit", FiniteOptions(2), false});
		cases.push_back({"finite-explicit-3", FiniteOptions(3), false});
		cases.push_back({"finite-explicit-4", FiniteOptions(4), false});
		for (const WorklistOrder order : {WorklistOrder::BreadthFirst,
			WorklistOrder::LowerCostFirst, WorklistOrder::Greedy}) {
			cases.push_back({"faithful-" + WorklistName(order), faithful(order),
				order == WorklistOrder::BreadthFirst});
		}
		return cases;
	}

	std::vector<Case> Cases(const Options& options) {
		auto all = AllCases();
		if (options.case_name) {
			const auto selected = std::ranges::find(all, *options.case_name, &Case::name);
			if (selected == all.end()) throw std::invalid_argument("unknown case: " + *options.case_name);
			return {*selected};
		}
		std::vector<std::string> selected;
		switch (options.suite) {
		case Suite::Smoke:
		case Suite::Validation:
			selected = {"faithful-breadth-first"};
			break;
		case Suite::Finite:
			selected = {"finite-explicit", "faithful-breadth-first"};
			break;
		case Suite::Worklists:
			selected = {"faithful-breadth-first", "faithful-lower-cost-first", "faithful-greedy"};
			break;
		case Suite::Scaling:
			selected = {"finite-explicit", "finite-explicit-3", "finite-explicit-4",
				"faithful-breadth-first"};
			break;
		case Suite::All:
			for (const auto& item : all) selected.push_back(item.name);
			break;
		}
		std::vector<Case> result;
		for (const auto& name : selected) {
			const auto item = std::ranges::find(all, name, &Case::name);
			if (item == all.end()) throw std::logic_error("paper suite references an unknown case");
			result.push_back(*item);
		}
		return result;
	}

	void List(const Options& options) {
		for (const auto& item : Cases(options)) std::cout << item.name << '\n';
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
				List(options);
				return 0;
			}
			const std::filesystem::path directory = OutputDirectory(options);
			std::ofstream metrics(directory / "metrics.tsv");
			metrics << "case\tstatus\toptimal_response_cost\tvalidated\tvalidation_worst_response\tvalidation_reachable_states\t"
				"fresh_cycles\tfresh_requests\tfresh_max_response\tfresh_max_active_domain\tactive_domain\trecipe_live\tfacility_live\t"
				"recipe_action_arity\tsupport\tfresh_edge\tpool\tarena_states\tarena_edges\tgroundings\tprogressions\t"
				"recipe_graph_states\trecipe_graph_edges\tresource_graph_states\tresource_graph_edges\tproduct_states\tproduct_edges\t"
				"isomorphism_checks\tisomorphism_matches\trejected_guards\trejected_preconditions\trejected_observations\t"
				"dominated_edges\tremoved_self_loops\tcallback_contract_samples\twinning_states\twinning_controller_states\t"
				"controller_strategy_entries\tqualitative_iterations\tbudget_tests\tgreedy_upper_bound\tkmax\t"
				"synthesis_ms\tarena_ms\tqualitative_ms\tbudget_ms\textraction_ms\tvalidation_ms\n";
			bool success = true;
			for (Case item : Cases(options)) {
				if (options.timeout) {
					item.options.deadline = std::chrono::steady_clock::now() + *options.timeout;
				}
				auto problem = examples::MakeAssemblyProblem();
				const auto synthesis_started = std::chrono::steady_clock::now();
				const SynthesisResult result = Synthesise(problem, item.options);
				const auto synthesis_duration = std::chrono::steady_clock::now() - synthesis_started;
				const auto& stats = result.statistics;
				ExerciseStatistics exercise;
				if (result.controller && item.exercise) {
					exercise = ExerciseFreshController(problem, *result.controller);
				}
				metrics << item.name << '\t' << StatusName(result.status) << '\t'
					<< result.optimal_response_cost.value_or(0) << '\t' << result.validation.valid << '\t'
					<< result.validation.worst_response_cost << '\t' << result.validation.reachable_states << '\t'
					<< exercise.cycles << '\t' << exercise.requests << '\t' << exercise.maximum_response_cost << '\t'
					<< exercise.maximum_active_domain << '\t'
					<< stats.bounds.active_domain << '\t' << stats.bounds.recipe_live << '\t'
					<< stats.bounds.facility_live << '\t' << stats.bounds.recipe_action_arity << '\t'
					<< stats.bounds.support << '\t' << stats.bounds.fresh_edge << '\t' << stats.bounds.pool << '\t'
					<< stats.arena_states << '\t' << stats.arena_edges << '\t'
					<< stats.arena.generated_substitutions << '\t' << stats.arena.progression_calls << '\t'
					<< stats.arena.recipe_graph_states << '\t' << stats.arena.recipe_graph_edges << '\t'
					<< stats.arena.resource_graph_states << '\t' << stats.arena.resource_graph_edges << '\t'
					<< stats.arena.product_states << '\t' << stats.arena.product_edges << '\t'
					<< stats.arena.isomorphism_checks << '\t' << stats.arena.isomorphism_matches << '\t'
					<< stats.arena.rejected_guards << '\t' << stats.arena.rejected_preconditions << '\t'
					<< stats.arena.rejected_observations << '\t' << stats.arena.dominated_edges << '\t'
					<< stats.arena.removed_self_loops << '\t' << stats.arena.callback_contract_samples << '\t'
					<< stats.winning_states << '\t' << stats.winning_controller_states << '\t'
					<< stats.controller_strategy_entries << '\t'
					<< stats.qualitative_iterations << '\t'
					<< stats.budget_tests << '\t' << stats.greedy_upper_bound << '\t'
					<< stats.theoretical_upper_bound << '\t' << std::setprecision(10)
					<< Milliseconds(synthesis_duration) << '\t'
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
				}
			}
			return success ? 0 : 1;
		} catch (const std::exception& error) {
			std::cerr << "scs_paper: " << error.what() << '\n';
			return 2;
		}
	}

}
