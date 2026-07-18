#include "experiments.h"

#include <fstream>
#include <iomanip>
#include <memory>
#include <random>
#include <stdexcept>

#include "Hinge/hinge.h"
#include "Hinge/Full/recipe.h"
#include "Hinge/Quick/recipe.h"
#include "Hinge/ExtendedGrounded/recipe.h"
#include "Hinge/ExtendedGrounded/resource_1.h"
#include "Hinge/ExtendedGrounded/resource_2.h"
#include "Hinge/ExtendedGrounded/resource_3.h"
#include "Hinge/ExtendedGrounded/resource_4.h"
#include "Hinge/ExtendedGrounded/resource_x.h"
#include "scs/Synthesis/synthesis.h"

namespace scs::paper {

	namespace {
		constexpr uint32_t kSeed = 2010;

		double CounterValue(const benchmark::UserCounters& counters, const std::string& name) {
			const auto found = counters.find(name);
			return found == counters.end() ? 0.0 : found->second.value;
		}

		std::string StatusName(double code) {
			if (code == 1.0) return "no_controller";
			if (code == 2.0) return "timeout";
			return "ok";
		}

		void SetReportCounters(benchmark::State& state, const SynthesisReport& report,
			int resources, const Limits& limits) {
			state.counters["resources"] = resources;
			state.counters["status_code"] = report.status == SynthesisStatus::Solved ? 0.0
				: report.status == SynthesisStatus::NoController ? 1.0 : 2.0;
			state.counters["visited_situations"] = static_cast<double>(report.statistics.visited_situations);
			state.counters["action_considerations"] = static_cast<double>(report.statistics.action_considerations);
			state.counters["cached_fluent_states"] = static_cast<double>(report.statistics.cached_fluent_states);
			state.counters["cache_hits"] = static_cast<double>(report.statistics.cache_hits);
			state.counters["topology_states"] = static_cast<double>(report.statistics.topology_states);
			state.counters["topology_transitions"] = static_cast<double>(report.statistics.topology_transitions);
			state.counters["global_transition_limit"] = limits.global_transition_limit;
			state.counters["global_cost_limit"] = limits.global_cost_limit;
			state.counters["stage_transition_limit"] = limits.stage_transition_limit;
			state.counters["stage_cost_limit"] = limits.stage_cost_limit;
			state.counters["fairness_limit"] = limits.fairness_limit;
			if (report.candidate) {
				state.counters["global_cost"] = report.candidate->total_cost;
				state.counters["total_transitions"] = report.candidate->total_transitions;
			}
		}

		std::filesystem::path ControllerPath(const std::filesystem::path& directory,
			const std::string& solver, int resources) {
			if (directory.empty()) return {};
			return directory / (solver + "-" + std::to_string(resources) + "-controller.gv");
		}

		void ExportCandidate(const SynthesisReport& report, const std::filesystem::path& path) {
			if (!path.empty() && report.candidate) {
				ExportControllerToFile(report.candidate->plan, path);
			}
		}

		struct ControllerScenario {
			std::vector<CharacteristicGraph> graphs;
			BasicActionTheory global;
			CharacteristicGraph recipe;
			std::unique_ptr<CompleteTopology> topology;
		};

		std::unique_ptr<ControllerScenario> BuildQuickScenario() {
			auto scenario = std::make_unique<ControllerScenario>();
			auto common_bat = examples::HingeCommonBAT();
			auto resource2 = examples::HingeResource2();
			auto resource4 = examples::HingeResource4();
			scenario->graphs.emplace_back(resource2.program, ProgramType::Resource);
			scenario->graphs.emplace_back(resource4.program, ProgramType::Resource);

			CoopMatrix cooperation(10);
			cooperation.Add(1, 2);
			cooperation.Add(1, 3);
			cooperation.Add(2, 3);
			RoutesMatrix routes(10);
			routes.Add(1, 2);
			routes.Add(2, 4);
			std::vector<BasicActionTheory> theories{common_bat, resource2.bat, resource4.bat};
			scenario->global = CombineBATs(theories, cooperation, routes);
			scenario->recipe = CharacteristicGraph(examples::HingeRecipeQuick(), ProgramType::Recipe);
			scenario->topology = std::make_unique<CompleteTopology>(&scenario->graphs, true);
			return scenario;
		}

		std::unique_ptr<ControllerScenario> BuildFullScenario(bool recursive) {
			auto scenario = std::make_unique<ControllerScenario>();
			auto common_bat = examples::HingeCommonBAT();
			auto resource1 = examples::HingeResource1();
			auto resource2 = examples::HingeResource2();
			auto resource3 = examples::HingeResource3();
			scenario->graphs.emplace_back(resource1.program, ProgramType::Resource);
			scenario->graphs.emplace_back(resource2.program, ProgramType::Resource);
			scenario->graphs.emplace_back(resource3.program, ProgramType::Resource);

			CoopMatrix cooperation(10);
			cooperation.Add(1, 2);
			cooperation.Add(1, 3);
			cooperation.Add(2, 3);
			RoutesMatrix routes(10);
			routes.Add(1, 2);
			std::vector<BasicActionTheory> theories{common_bat, resource1.bat, resource2.bat, resource3.bat};
			scenario->global = CombineBATs(theories, cooperation, routes);
			scenario->recipe = CharacteristicGraph(examples::HingeRecipe(), ProgramType::Recipe);
			scenario->topology = std::make_unique<CompleteTopology>(&scenario->graphs, recursive);
			return scenario;
		}

		struct ScalingScenario {
			std::vector<CharacteristicGraph> graphs;
			std::vector<BasicActionTheory> theories;
			CharacteristicGraph recipe;
			CoopMatrix cooperation{10};
			RoutesMatrix routes{10};
			Limits limits;
		};

		std::unique_ptr<ScalingScenario> BuildScalingScenario(int resources) {
			auto scenario = std::make_unique<ScalingScenario>();
			scenario->cooperation.Add(1, 2);
			scenario->cooperation.Add(1, 3);
			scenario->cooperation.Add(2, 3);
			scenario->cooperation.Add(1, 4);
			scenario->cooperation.Add(2, 4);
			scenario->cooperation.Add(5, 1);
			scenario->cooperation.Add(4, 1);
			scenario->cooperation.Add(3, 1);
			scenario->cooperation.Add(2, 1);
			scenario->routes.Add(1, 2);
			scenario->routes.Add(2, 4);
			scenario->routes.Add(1, 4);
			scenario->theories.emplace_back(examples::HingeCommonBAT());

			if (resources == 2) {
				auto resource2 = examples::HingeGroundedResource2();
				auto resource4 = examples::HingeGroundedResource4();
				scenario->graphs.emplace_back(resource2.program, ProgramType::Resource);
				scenario->graphs.emplace_back(examples::HingeGroundedResource4Cg());
				scenario->theories.push_back(resource2.bat);
				scenario->theories.push_back(resource4.bat);
				scenario->recipe = CharacteristicGraph(examples::HingeRecipeQuick(), ProgramType::Recipe);
				scenario->limits = Limits{10, 200, 3, 50, 20};
				return scenario;
			}

			auto resource1 = examples::HingeGroundedResource1();
			auto resource2 = examples::HingeGroundedResource2();
			auto resource3 = examples::HingeGroundedResource3();
			scenario->graphs.emplace_back(examples::HingeGroundedResource1Cg());
			scenario->graphs.emplace_back(resource2.program, ProgramType::Resource);
			scenario->graphs.emplace_back(resource3.program, ProgramType::Resource);
			scenario->theories.push_back(resource1.bat);
			scenario->theories.push_back(resource2.bat);
			scenario->theories.push_back(resource3.bat);
			if (resources >= 4) {
				auto resource4 = examples::HingeGroundedResource4();
				scenario->graphs.emplace_back(examples::HingeGroundedResource4Cg());
				scenario->theories.push_back(resource4.bat);
			}
			if (resources > 4) {
				auto resource_x = examples::HingeGroundedResourceX();
				for (int i = 4; i < resources; ++i) {
					scenario->graphs.emplace_back(resource_x.program, ProgramType::Resource);
				}
				scenario->theories.push_back(resource_x.bat);
			}
			scenario->recipe = CharacteristicGraph(resources == 3 ? examples::HingeRecipe()
				: examples::HingeExtendedGroundedRecipe(), ProgramType::Recipe);
			scenario->limits = Limits{2048, 8192, 50, 500, 20};
			return scenario;
		}

		void RegisterGrounding() {
			for (int count = 1; count <= 3; ++count) {
				auto* registration = benchmark::RegisterBenchmark(
					("paper/grounding/" + std::to_string(count)).c_str(), [count](benchmark::State& state) {
					size_t instantiations = 0;
					for (auto _ : state) {
						(void)_;
						auto resource1 = examples::HingeResource1();
						auto resource2 = examples::HingeResource2();
						auto resource3 = examples::HingeResource3();
						std::vector<CharacteristicGraph> graphs;
						graphs.emplace_back(resource1.program, ProgramType::Resource);
						if (count >= 2) graphs.emplace_back(resource2.program, ProgramType::Resource);
						if (count >= 3) graphs.emplace_back(resource3.program, ProgramType::Resource);
						ankerl::unordered_dense::set<Object> objects{
							"o1", "o2", "o3", "o4", "o5", "o6", "o7", "o8", "o9", "o10", "o11", "o12"};
						CompleteTopology topology(&graphs, true);
						auto cache = PreExpand(topology, objects);
						instantiations = cache.SizeComplete();
						benchmark::DoNotOptimize(cache);
					}
					state.counters["resources"] = count;
					state.counters["compound_action_instantiations"] = static_cast<double>(instantiations);
				});
				registration->Unit(benchmark::kSecond)->MinTime(0.5);
			}
		}

		void RegisterQuickControllers(const std::filesystem::path& controller_directory) {
			const Limits astar_limits{10, 200, 3, 50, 20};
			const auto astar_path = ControllerPath(controller_directory, "astar", 2);
			auto* astar = benchmark::RegisterBenchmark("paper/astar/2", [astar_limits, astar_path](benchmark::State& state) {
				SynthesisReport last;
				for (auto _ : state) {
					(void)_;
					state.PauseTiming();
					auto scenario = BuildQuickScenario();
					auto solver = std::make_unique<AStar>(scenario->graphs, scenario->recipe,
						scenario->global, *scenario->topology, astar_limits);
					state.ResumeTiming();
					last = solver->Synthesise();
					state.PauseTiming();
					benchmark::DoNotOptimize(last.candidate);
					solver.reset();
					scenario.reset();
					state.ResumeTiming();
				}
				SetReportCounters(state, last, 2, astar_limits);
				ExportCandidate(last, astar_path);
			});
			astar->Unit(benchmark::kSecond)->Iterations(1000);

			const Limits greedy_limits{50, 200, 4, 50, 20};
			const auto greedy_path = ControllerPath(controller_directory, "gbfs", 2);
			auto* greedy = benchmark::RegisterBenchmark("paper/gbfs/2", [greedy_limits, greedy_path](benchmark::State& state) {
				SynthesisReport last;
				for (auto _ : state) {
					(void)_;
					state.PauseTiming();
					auto scenario = BuildQuickScenario();
					auto solver = std::make_unique<GS>(scenario->graphs, scenario->recipe,
						scenario->global, *scenario->topology, greedy_limits, false, std::mt19937{kSeed});
					state.ResumeTiming();
					last = solver->Synthesise();
					state.PauseTiming();
					benchmark::DoNotOptimize(last.candidate);
					solver.reset();
					scenario.reset();
					state.ResumeTiming();
				}
				SetReportCounters(state, last, 2, greedy_limits);
				ExportCandidate(last, greedy_path);
			});
			greedy->Unit(benchmark::kSecond)->Iterations(1000);
		}

		void RegisterFullGreedy(const std::filesystem::path& controller_directory) {
			const Limits limits{2048, 8192, 50, 500, 20};
			const auto controller_path = ControllerPath(controller_directory, "gbfs", 3);
			auto* registration = benchmark::RegisterBenchmark("paper/gbfs/3", [limits, controller_path](benchmark::State& state) {
				SynthesisReport last;
				for (auto _ : state) {
					(void)_;
					state.PauseTiming();
					auto scenario = BuildFullScenario(true);
					auto solver = std::make_unique<GS>(scenario->graphs, scenario->recipe,
						scenario->global, *scenario->topology, limits, true, std::mt19937{kSeed});
					state.ResumeTiming();
					last = solver->Synthesise();
					state.PauseTiming();
					benchmark::DoNotOptimize(last.candidate);
					solver.reset();
					scenario.reset();
					state.ResumeTiming();
				}
				SetReportCounters(state, last, 3, limits);
				ExportCandidate(last, controller_path);
			});
			registration->Unit(benchmark::kSecond)->MinTime(0.5);
		}

		void RegisterLimits() {
			const std::vector<int> costs{25, 50, 75, 100, 150, 200, 250, 300, 350, 400};
			for (const int cost : costs) {
				auto* registration = benchmark::RegisterBenchmark(
					("paper/phase_cost/" + std::to_string(cost)).c_str(), [cost](benchmark::State& state) {
					const Limits limits{2048, 8192, 50, cost, 20};
					SynthesisReport last;
					for (auto _ : state) {
						(void)_;
						state.PauseTiming();
						auto scenario = BuildFullScenario(false);
						state.ResumeTiming();
						GS solver(scenario->graphs, scenario->recipe, scenario->global,
							*scenario->topology, limits, true, std::mt19937{kSeed});
						last = solver.Synthesise();
						benchmark::DoNotOptimize(last.candidate);
					}
					SetReportCounters(state, last, 3, limits);
					state.counters["parameter"] = cost;
				});
				registration->Unit(benchmark::kSecond)->Iterations(5);
			}

			const std::vector<int> transitions{3, 4, 5, 6, 7, 8, 9, 10, 20, 30, 40, 50};
			for (const int transition_limit : transitions) {
				auto* registration = benchmark::RegisterBenchmark(
					("paper/phase_transitions/" + std::to_string(transition_limit)).c_str(),
					[transition_limit](benchmark::State& state) {
					const Limits limits{2048, 8192, transition_limit, 500, 20};
					SynthesisReport last;
					for (auto _ : state) {
						(void)_;
						state.PauseTiming();
						auto scenario = BuildFullScenario(false);
						state.ResumeTiming();
						GS solver(scenario->graphs, scenario->recipe, scenario->global,
							*scenario->topology, limits, true, std::mt19937{kSeed});
						last = solver.Synthesise();
						benchmark::DoNotOptimize(last.candidate);
					}
					SetReportCounters(state, last, 3, limits);
					state.counters["parameter"] = transition_limit;
				});
				registration->Unit(benchmark::kSecond)->Iterations(5);
			}
		}

		void RegisterScaling() {
			for (int resources = 2; resources <= 6; ++resources) {
				auto* registration = benchmark::RegisterBenchmark(
					("paper/scaling/" + std::to_string(resources)).c_str(), [resources](benchmark::State& state) {
					SynthesisReport last;
					Limits limits;
					for (auto _ : state) {
						(void)_;
						state.PauseTiming();
						auto scenario = BuildScalingScenario(resources);
						limits = scenario->limits;
						state.ResumeTiming();
						auto global = CombineBATs(scenario->theories, scenario->cooperation, scenario->routes);
						CompleteTopology topology(&scenario->graphs);
						GS solver(scenario->graphs, scenario->recipe, global, topology,
							limits, true, std::mt19937{kSeed});
						last = solver.Synthesise();
						benchmark::DoNotOptimize(last.candidate);
					}
					SetReportCounters(state, last, resources, limits);
				});
				registration->Unit(benchmark::kSecond)->Iterations(1);
			}
		}

		void RegisterThreeResourceAStar(std::chrono::milliseconds timeout,
			const std::filesystem::path& snapshot_path,
			const std::filesystem::path& controller_directory) {
			const Limits limits{50, 200, 4, 50, 20};
			const auto controller_path = ControllerPath(controller_directory, "astar", 3);
			auto* registration = benchmark::RegisterBenchmark("paper/astar/3", [=](benchmark::State& state) {
				SynthesisReport last;
				for (auto _ : state) {
					(void)_;
					state.PauseTiming();
					auto scenario = BuildFullScenario(true);
					auto started = std::chrono::steady_clock::now();
					SearchControl control;
					control.deadline = started + timeout;
					control.on_best_candidate = [=](const Candidate& candidate, const SynthesisStatistics& statistics) {
						ResultRow snapshot;
						snapshot.experiment = "astar";
						snapshot.parameter = 3;
						snapshot.status = "running";
						snapshot.wall_seconds = std::chrono::duration<double>(
							std::chrono::steady_clock::now() - started).count();
						snapshot.values["visited_situations"] = static_cast<double>(statistics.visited_situations);
						snapshot.values["global_cost"] = candidate.total_cost;
						snapshot.values["total_transitions"] = candidate.total_transitions;
						WriteWorkerResult(snapshot_path, snapshot);
					};
					auto solver = std::make_unique<AStar>(scenario->graphs, scenario->recipe,
						scenario->global, *scenario->topology, limits);
					state.ResumeTiming();
					last = solver->Synthesise(control);
					state.PauseTiming();
					benchmark::DoNotOptimize(last.candidate);
					solver.reset();
					scenario.reset();
					state.ResumeTiming();
				}
				SetReportCounters(state, last, 3, limits);
				ExportCandidate(last, controller_path);
			});
			registration->Unit(benchmark::kSecond)->Iterations(1);
		}
	}

	CollectingReporter::CollectingReporter(std::function<void(const ResultRow&)> on_row)
		: ConsoleReporter(benchmark::ConsoleReporter::OO_Tabular), on_row_(std::move(on_row)) {}

	void CollectingReporter::ReportRuns(const std::vector<Run>& reports) {
		ConsoleReporter::ReportRuns(reports);
		for (const auto& run : reports) {
			if (run.run_type != Run::RT_Iteration) continue;
			const std::string name = run.run_name.function_name;
			if (!name.starts_with("paper/")) continue;
			const auto first = name.find('/', 6);
			if (first == std::string::npos) continue;
			ResultRow row;
			row.experiment = name.substr(6, first - 6);
			row.parameter = std::stoi(name.substr(first + 1));
			row.status = StatusName(CounterValue(run.counters, "status_code"));
			row.iterations = run.iterations;
			row.cpu_seconds = run.cpu_accumulated_time / static_cast<double>(std::max<int64_t>(1, run.iterations));
			row.wall_seconds = run.real_accumulated_time / static_cast<double>(std::max<int64_t>(1, run.iterations));
			for (const auto& [key, counter] : run.counters) {
				row.values[key] = counter.value;
			}
			if (on_row_) on_row_(row);
			rows_.push_back(std::move(row));
		}
	}

	void RegisterExperiments(Suite suite, bool include_three_resource_astar,
		std::chrono::milliseconds astar_timeout, const std::filesystem::path& snapshot_path,
		const std::filesystem::path& controller_directory) {
		if (suite == Suite::All || suite == Suite::Tables || suite == Suite::Grounding) RegisterGrounding();
		if (suite == Suite::All || suite == Suite::Tables || suite == Suite::Controllers) {
			RegisterQuickControllers(controller_directory);
		}
		if (suite == Suite::All || suite == Suite::Tables || suite == Suite::Controllers) {
			RegisterFullGreedy(controller_directory);
		}
		if (include_three_resource_astar || suite == Suite::AStarWorker)
			RegisterThreeResourceAStar(astar_timeout, snapshot_path, controller_directory);
		if (suite == Suite::All || suite == Suite::Limits) RegisterLimits();
		if (suite == Suite::All || suite == Suite::Scaling) RegisterScaling();
	}

	std::string BenchmarkFilter(Suite suite) {
		switch (suite) {
		case Suite::Grounding: return "paper/grounding/.*";
		case Suite::Controllers: return "paper/(astar|gbfs)/.*";
		case Suite::AStarWorker: return "paper/astar/3";
		case Suite::Limits: return "paper/phase_.*";
		case Suite::Scaling: return "paper/scaling/.*";
		default: return "paper/.*";
		}
	}

	void WriteWorkerResult(const std::filesystem::path& path, const ResultRow& row) {
		if (path.empty()) return;
		const auto temporary = path.string() + ".tmp";
		std::ofstream output(temporary, std::ios::trunc);
		if (!output) return;
		output << "status\titerations\tcpu_seconds\twall_seconds\tvisited_situations\tglobal_cost\ttotal_transitions\n";
		const auto value_or_zero = [&row](const std::string& name) {
			const auto found = row.values.find(name);
			return found == row.values.end() ? 0.0 : found->second;
		};
		output << row.status << '\t' << row.iterations << '\t' << std::setprecision(17)
			<< row.cpu_seconds << '\t' << row.wall_seconds << '\t'
			<< value_or_zero("visited_situations") << '\t'
			<< value_or_zero("global_cost") << '\t'
			<< value_or_zero("total_transitions") << '\n';
		output.close();
		std::error_code error;
		std::filesystem::remove(path, error);
		std::filesystem::rename(temporary, path, error);
	}

	std::optional<ResultRow> ReadWorkerResult(const std::filesystem::path& path) {
		std::ifstream input(path);
		std::string header;
		std::string line;
		if (!std::getline(input, header) || !std::getline(input, line)) return std::nullopt;
		std::vector<std::string> fields;
		size_t start = 0;
		while (true) {
			const auto separator = line.find('\t', start);
			fields.push_back(line.substr(start, separator - start));
			if (separator == std::string::npos) break;
			start = separator + 1;
		}
		if (fields.size() != 7) return std::nullopt;
		ResultRow row;
		row.experiment = "astar";
		row.parameter = 3;
		row.status = fields[0];
		row.iterations = std::stoll(fields[1]);
		row.cpu_seconds = std::stod(fields[2]);
		row.wall_seconds = std::stod(fields[3]);
		row.values["visited_situations"] = std::stod(fields[4]);
		row.values["global_cost"] = std::stod(fields[5]);
		row.values["total_transitions"] = std::stod(fields[6]);
		return row;
	}

}
