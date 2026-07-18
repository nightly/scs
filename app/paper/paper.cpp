#include "paper.h"

#include <benchmark/benchmark.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "experiments.h"
#include "process.h"

namespace scs::paper {

	namespace {
		constexpr uint32_t kSeed = 2010;

		struct Options {
			Suite suite = Suite::All;
			std::optional<std::filesystem::path> output_directory;
			std::chrono::milliseconds astar_timeout = std::chrono::hours(3);
			std::vector<int> scaling_resources{kDefaultScalingResources.begin(),
				kDefaultScalingResources.end()};
			bool list = false;
			bool help = false;
		};

		double Value(const ResultRow& row, const std::string& key) {
			const auto found = row.values.find(key);
			return found == row.values.end() ? 0.0 : found->second;
		}

		long long IntegerValue(const ResultRow& row, const std::string& key) {
			return std::llround(Value(row, key));
		}

		class TsvWriter {
		public:
			explicit TsvWriter(const std::filesystem::path& directory) : directory_(directory) {
				Open("grounding", "resources\tstatus\titerations\tcpu_time_s\twall_time_s\tcompound_action_instantiations\n");
				Open("astar", ControllerHeader());
				Open("gbfs", ControllerHeader());
				Open("phase_cost", LimitHeader("stage_cost_limit"));
				Open("phase_transitions", LimitHeader("stage_transition_limit"));
				Open("scaling", "resources\tactive_resources\tstatus\titerations\tcpu_time_s\twall_time_s\t"
					"visited_situations\taction_considerations\tcached_fluent_states\tcache_hits\t"
					"topology_states\ttopology_transitions\tglobal_cost\ttotal_transitions\t"
					"global_cost_limit\tstage_cost_limit\n");
			}

			void Write(const ResultRow& row) {
				auto found = files_.find(row.experiment);
				if (found == files_.end()) return;
				auto& output = found->second;
				output << std::setprecision(10);
				if (row.experiment == "grounding") {
					output << row.parameter << '\t' << row.status << '\t' << row.iterations << '\t'
						<< row.cpu_seconds << '\t' << row.wall_seconds << '\t'
						<< IntegerValue(row, "compound_action_instantiations") << '\n';
				} else if (row.experiment == "astar" || row.experiment == "gbfs") {
					output << row.parameter << '\t' << row.status << '\t' << row.iterations << '\t'
						<< row.cpu_seconds * 1000.0 << '\t' << row.wall_seconds * 1000.0 << '\t'
						<< IntegerValue(row, "visited_situations") << '\t'
						<< IntegerValue(row, "global_cost") << '\t'
						<< IntegerValue(row, "total_transitions") << '\t'
						<< IntegerValue(row, "global_transition_limit") << '\t'
						<< IntegerValue(row, "global_cost_limit") << '\t'
						<< IntegerValue(row, "stage_transition_limit") << '\t'
						<< IntegerValue(row, "stage_cost_limit") << '\t'
						<< IntegerValue(row, "fairness_limit") << '\n';
				} else if (row.experiment == "phase_cost" || row.experiment == "phase_transitions") {
					output << row.parameter << '\t' << row.status << '\t' << row.iterations << '\t'
						<< row.cpu_seconds << '\t' << row.wall_seconds << '\t'
						<< IntegerValue(row, "visited_situations") << '\t'
						<< IntegerValue(row, "global_cost") << '\t'
						<< IntegerValue(row, "total_transitions") << '\n';
				} else if (row.experiment == "scaling") {
					output << row.parameter << '\t' << IntegerValue(row, "active_resources") << '\t'
						<< row.status << '\t' << row.iterations << '\t'
						<< row.cpu_seconds << '\t' << row.wall_seconds << '\t'
						<< IntegerValue(row, "visited_situations") << '\t'
						<< IntegerValue(row, "action_considerations") << '\t'
						<< IntegerValue(row, "cached_fluent_states") << '\t'
						<< IntegerValue(row, "cache_hits") << '\t'
						<< IntegerValue(row, "topology_states") << '\t'
						<< IntegerValue(row, "topology_transitions") << '\t'
						<< IntegerValue(row, "global_cost") << '\t'
						<< IntegerValue(row, "total_transitions") << '\t'
						<< IntegerValue(row, "global_cost_limit") << '\t'
						<< IntegerValue(row, "stage_cost_limit") << '\n';
				}
				output.flush();
				std::cout << "[paper] wrote " << row.experiment << " case " << row.parameter
					<< " (" << row.status << ")\n";
			}

		private:
			std::filesystem::path directory_;
			std::map<std::string, std::ofstream> files_;

			static std::string ControllerHeader() {
				return "resources\tstatus\titerations\tcpu_time_ms\twall_time_ms\t"
					"visited_situations\tglobal_cost\ttotal_transitions\tglobal_transition_limit\t"
					"global_cost_limit\tstage_transition_limit\tstage_cost_limit\tfairness_limit\n";
			}

			static std::string LimitHeader(const std::string& parameter) {
				return parameter + "\tstatus\titerations\tcpu_time_s\twall_time_s\tvisited_situations\tglobal_cost\ttotal_transitions\n";
			}

			void Open(const std::string& name, const std::string& header) {
				auto [entry, inserted] = files_.try_emplace(name, directory_ / (name + ".tsv"), std::ios::trunc);
				if (!inserted || !entry->second) {
					throw std::runtime_error("Unable to create " + name + ".tsv");
				}
				entry->second << header;
				entry->second.flush();
			}
		};

		std::chrono::milliseconds ParseDuration(const std::string& text) {
			if (text.empty()) throw std::invalid_argument("duration cannot be empty");
			size_t parsed = 0;
			const double value = std::stod(text, &parsed);
			if (value <= 0.0) throw std::invalid_argument("duration must be positive");
			const std::string suffix = text.substr(parsed);
			double milliseconds = 0.0;
			if (suffix == "ms") milliseconds = value;
			else if (suffix == "s") milliseconds = value * 1000.0;
			else if (suffix == "m") milliseconds = value * 60'000.0;
			else if (suffix == "h") milliseconds = value * 3'600'000.0;
			else throw std::invalid_argument("duration must use ms, s, m, or h");
			return std::chrono::milliseconds(static_cast<int64_t>(milliseconds));
		}

		std::vector<int> ParseResourceCounts(const std::string& text) {
			if (text.empty()) throw std::invalid_argument("resource counts cannot be empty");
			std::vector<int> counts;
			size_t start = 0;
			while (start <= text.size()) {
				const auto separator = text.find(',', start);
				const auto field = text.substr(start, separator - start);
				size_t parsed = 0;
				const int count = std::stoi(field, &parsed);
				if (parsed != field.size() || count < 2) {
					throw std::invalid_argument("scaling resource counts must be integers of at least 2");
				}
				counts.push_back(count);
				if (separator == std::string::npos) break;
				start = separator + 1;
			}
			std::sort(counts.begin(), counts.end());
			counts.erase(std::unique(counts.begin(), counts.end()), counts.end());
			return counts;
		}

		Suite ParseSuite(const std::string& value) {
			if (value == "all") return Suite::All;
			if (value == "tables") return Suite::Tables;
			if (value == "grounding") return Suite::Grounding;
			if (value == "controllers") return Suite::Controllers;
			if (value == "limits") return Suite::Limits;
			if (value == "scaling") return Suite::Scaling;
			throw std::invalid_argument("unknown suite: " + value);
		}

		Options ParseOptions(int argc, char** argv) {
			Options options;
			for (int i = 1; i < argc; ++i) {
				const std::string argument = argv[i];
				const auto require_value = [&](const std::string& name) -> std::string {
					if (++i >= argc) throw std::invalid_argument(name + " requires a value");
					return argv[i];
				};
				if (argument == "--suite") options.suite = ParseSuite(require_value(argument));
				else if (argument == "--output-dir") options.output_directory = require_value(argument);
				else if (argument == "--astar-timeout") options.astar_timeout = ParseDuration(require_value(argument));
				else if (argument == "--scaling-resources") {
					options.scaling_resources = ParseResourceCounts(require_value(argument));
				}
				else if (argument == "--list") options.list = true;
				else if (argument == "--help" || argument == "-h") options.help = true;
				else throw std::invalid_argument("unknown argument: " + argument);
			}
			return options;
		}

		void PrintUsage(std::ostream& output) {
			output << "Usage: scs_paper [--suite all|tables|grounding|controllers|limits|scaling]\n"
				"                 [--output-dir PATH] [--astar-timeout 3h]\n"
				"                 [--scaling-resources 2,3,4,8,...] [--list]\n\n"
				"The default full suite can take several hours. Results are written as TSV.\n";
		}

		void PrintCases(Suite suite, std::span<const int> scaling_resources) {
			const auto show = [suite](Suite group) {
				return suite == Suite::All || suite == group
					|| (suite == Suite::Tables && (group == Suite::Grounding || group == Suite::Controllers));
			};
			if (show(Suite::Grounding)) std::cout << "grounding: resources 1,2,3\n";
			if (show(Suite::Controllers)) std::cout << "astar: resources 2,3; gbfs: resources 2,3\n";
			if (show(Suite::Limits)) {
				std::cout << "phase-cost: 25,50,75,100,150,200,250,300,350,400\n";
				std::cout << "phase-transitions: 3,4,5,6,7,8,9,10,20,30,40,50\n";
			}
			if (show(Suite::Scaling)) {
				std::cout << "scaling: resources ";
				for (size_t i = 0; i < scaling_resources.size(); ++i) {
					if (i != 0) std::cout << ',';
					std::cout << scaling_resources[i];
				}
				std::cout << " (three active from total >= 3)\n";
			}
		}

		std::string Timestamp() {
			const auto now = std::chrono::system_clock::now();
			const std::time_t time = std::chrono::system_clock::to_time_t(now);
			std::tm utc{};
#ifdef _WIN32
			gmtime_s(&utc, &time);
#else
			gmtime_r(&time, &utc);
#endif
			std::ostringstream result;
			result << std::put_time(&utc, "%Y%m%dT%H%M%SZ");
			return result.str();
		}

		std::filesystem::path PrepareOutputDirectory(const Options& options) {
			std::filesystem::path directory;
			if (options.output_directory) {
				directory = *options.output_directory;
				if (std::filesystem::exists(directory)
					&& std::filesystem::directory_iterator(directory) != std::filesystem::directory_iterator{}) {
					throw std::runtime_error("output directory is not empty: " + directory.string());
				}
			} else {
				const auto base = std::filesystem::path("exports") / "paper-results";
				directory = base / Timestamp();
				int suffix = 1;
				while (std::filesystem::exists(directory)) {
					directory = base / (Timestamp() + "-" + std::to_string(suffix++));
				}
			}
			std::filesystem::create_directories(directory);
			return std::filesystem::absolute(directory);
		}

		std::string CompilerName() {
#if defined(__clang__)
			return std::string("Clang ") + __clang_version__;
#elif defined(__GNUC__)
			return std::string("GCC ") + __VERSION__;
#elif defined(_MSC_VER)
			return std::string("MSVC ") + std::to_string(_MSC_VER);
#else
			return "unknown";
#endif
		}

		std::string OperatingSystem() {
#if defined(_WIN32)
			return "Windows";
#elif defined(__APPLE__)
			return "macOS";
#elif defined(__linux__)
			return "Linux";
#else
			return "unknown";
#endif
		}

		void WriteRunMetadata(const std::filesystem::path& directory, const Options& options,
			const std::string& status, bool append) {
			std::ofstream output(directory / "run.tsv", append ? std::ios::app : std::ios::trunc);
			if (!output) throw std::runtime_error("Unable to write run.tsv");
			if (!append) output << "key\tvalue\n";
			output << (append ? "finished_at" : "started_at") << '\t' << Timestamp() << '\n';
			output << (append ? "final_status" : "status") << '\t' << status << '\n';
			if (!append) {
				output << "suite\t" << SuiteName(options.suite) << '\n';
				output << "seed\t" << kSeed << '\n';
				output << "astar_timeout_ms\t" << options.astar_timeout.count() << '\n';
				output << "scaling_resources\t";
				for (size_t i = 0; i < options.scaling_resources.size(); ++i) {
					if (i != 0) output << ',';
					output << options.scaling_resources[i];
				}
				output << '\n';
				output << "compiler\t" << CompilerName() << '\n';
				output << "operating_system\t" << OperatingSystem() << '\n';
				output << "hardware_threads\t" << std::thread::hardware_concurrency() << '\n';
#ifdef SCS_BUILD_TYPE
				output << "build_type\t" << SCS_BUILD_TYPE << '\n';
#else
				output << "build_type\tunknown\n";
#endif
#ifdef SCS_GIT_COMMIT
				output << "git_commit\t" << SCS_GIT_COMMIT << '\n';
#else
				output << "git_commit\tunknown\n";
#endif
			}
		}

		void InitializeBenchmark(const char* executable) {
			int benchmark_argc = 1;
			char* benchmark_argv[] = {const_cast<char*>(executable), nullptr};
			benchmark::Initialize(&benchmark_argc, benchmark_argv);
		}

		bool IncludesControllers(Suite suite) {
			return suite == Suite::All || suite == Suite::Tables || suite == Suite::Controllers;
		}

		void AddAStarLimits(ResultRow& row) {
			row.values["global_transition_limit"] = 50;
			row.values["global_cost_limit"] = 200;
			row.values["stage_transition_limit"] = 4;
			row.values["stage_cost_limit"] = 50;
			row.values["fairness_limit"] = 20;
		}

		int RunWorker(int argc, char** argv) {
			spdlog::set_level(spdlog::level::warn);
			std::filesystem::path output;
			std::chrono::milliseconds timeout{0};
			for (int i = 3; i < argc; ++i) {
				const std::string argument = argv[i];
				if (argument == "--output" && ++i < argc) output = argv[i];
				else if (argument == "--timeout-ms" && ++i < argc)
					timeout = std::chrono::milliseconds(std::stoll(argv[i]));
				else throw std::invalid_argument("invalid internal worker argument");
			}
			if (output.empty() || timeout <= std::chrono::milliseconds::zero())
				throw std::invalid_argument("internal worker requires output and timeout");

			InitializeBenchmark(argv[0]);
			RegisterExperiments(Suite::AStarWorker, true, timeout, output, output.parent_path());
			CollectingReporter reporter([&output](const ResultRow& row) { WriteWorkerResult(output, row); });
			benchmark::RunSpecifiedBenchmarks(&reporter, BenchmarkFilter(Suite::AStarWorker));
			if (reporter.Rows().empty()) return 2;
			return reporter.Rows().front().status == "no_controller" ? 3 : 0;
		}
	}

	std::string_view SuiteName(Suite suite) {
		switch (suite) {
		case Suite::All: return "all";
		case Suite::Tables: return "tables";
		case Suite::Grounding: return "grounding";
		case Suite::Controllers: return "controllers";
		case Suite::Limits: return "limits";
		case Suite::Scaling: return "scaling";
		case Suite::AStarWorker: return "astar-worker";
		}
		return "unknown";
	}

	int Run(int argc, char** argv) {
		try {
			spdlog::set_level(spdlog::level::warn);
			if (argc >= 2 && std::string_view(argv[1]) == "--internal-worker") {
				if (argc < 3 || std::string_view(argv[2]) != "astar3") return 2;
				return RunWorker(argc, argv);
			}

			const Options options = ParseOptions(argc, argv);
			if (options.help) {
				PrintUsage(std::cout);
				return 0;
			}
			if (options.list) {
				PrintCases(options.suite, options.scaling_resources);
				return 0;
			}

			const auto output_directory = PrepareOutputDirectory(options);
			WriteRunMetadata(output_directory, options, "running", false);
			TsvWriter writer(output_directory);
			std::cout << "[paper] output: " << output_directory << '\n';
#ifndef NDEBUG
			std::cerr << "[paper] warning: this is a Debug build; use Release for meaningful timings\n";
#endif
			if (options.suite == Suite::All) {
				std::cout << "[paper] the full suite includes a three-hour A* timeout and may take several hours\n";
			}

			InitializeBenchmark(argv[0]);
			RegisterExperiments(options.suite, false, options.astar_timeout, {}, output_directory,
				options.scaling_resources);
			bool failed = false;
			CollectingReporter reporter([&](const ResultRow& row) {
				writer.Write(row);
				if (row.status == "no_controller") failed = true;
			});
			benchmark::RunSpecifiedBenchmarks(&reporter, BenchmarkFilter(options.suite));

			if (IncludesControllers(options.suite)) {
				std::cout << "[paper] starting supervised three-resource A* attempt (timeout "
					<< options.astar_timeout.count() << " ms)\n";
				const auto worker_result = output_directory / "astar-worker.tsv";
				const auto executable = std::filesystem::absolute(argv[0]);
				const auto child = RunChild(executable,
					{"--internal-worker", "astar3", "--output", worker_result.string(),
					 "--timeout-ms", std::to_string(options.astar_timeout.count())},
					options.astar_timeout);
				auto row = ReadWorkerResult(worker_result).value_or(ResultRow{});
				row.experiment = "astar";
				row.parameter = 3;
				if (child.killed) row.status = "killed";
				else if (child.exit_code != 0 && row.status.empty()) row.status = "error";
				if (row.status == "running") row.status = child.killed ? "killed" : "timeout";
				if (row.status.empty() || (row.status == "ok" && child.exit_code != 0)) row.status = "error";
				if (row.wall_seconds == 0.0) row.wall_seconds = child.wall_seconds;
				AddAStarLimits(row);
				writer.Write(row);
				std::error_code ignored;
				std::filesystem::remove(worker_result, ignored);
				if (row.status == "error" || row.status == "killed") failed = true;
			}

			WriteRunMetadata(output_directory, options, failed ? "failed" : "completed", true);
			std::cout << "[paper] " << (failed ? "completed with errors" : "completed") << '\n';
			return failed ? 1 : 0;
		} catch (const std::exception& error) {
			std::cerr << "scs_paper: " << error.what() << '\n';
			PrintUsage(std::cerr);
			return 2;
		}
	}

}
