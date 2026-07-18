#pragma once

#include <benchmark/benchmark.h>

#include <array>
#include <chrono>
#include <filesystem>
#include <functional>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "paper.h"

namespace scs::paper {

	inline constexpr std::array kDefaultScalingResources{
		2, 3, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048};

	struct ResultRow {
		std::string experiment;
		int parameter = 0;
		std::string status = "ok";
		int64_t iterations = 0;
		double cpu_seconds = 0.0;
		double wall_seconds = 0.0;
		std::map<std::string, double> values;
	};

	void WriteWorkerResult(const std::filesystem::path& path, const ResultRow& row);
	std::optional<ResultRow> ReadWorkerResult(const std::filesystem::path& path);

	class CollectingReporter final : public benchmark::ConsoleReporter {
	public:
		explicit CollectingReporter(std::function<void(const ResultRow&)> on_row = {});
		void ReportRuns(const std::vector<Run>& reports) override;
		const std::vector<ResultRow>& Rows() const { return rows_; }
	private:
		std::vector<ResultRow> rows_;
		std::function<void(const ResultRow&)> on_row_;
	};

	void RegisterExperiments(Suite suite, bool include_three_resource_astar = false,
		std::chrono::milliseconds astar_timeout = std::chrono::hours(3),
		const std::filesystem::path& snapshot_path = {},
		const std::filesystem::path& controller_directory = {},
		std::span<const int> scaling_resources = kDefaultScalingResources);
	std::string BenchmarkFilter(Suite suite);

}
