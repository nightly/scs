#include <benchmark/benchmark.h>
#include "scs/Combinatorics/CartesianProduct/product.h"

using namespace scs;

static void Instance0(benchmark::State& state) {
	std::vector v1 = { 10, 20, 5 };
	std::vector v2 = { 2, 10 };
	std::vector v3 = { 9 };
	std::vector v4 = { 50, 25, 100 };
	std::vector v5 = { 1000, 2000, 3000, 4000, 5000, 6000 };
	std::vector<const std::vector<int>*> vecs = { &v1, &v2, &v3, &v4, &v5};

	for (auto _ : state) {
		auto product = Product(vecs);
		benchmark::DoNotOptimize(product);
		benchmark::ClobberMemory();
	}
}
BENCHMARK(Instance0)->Unit(benchmark::kMillisecond);
// 0.006 ms

static void Instance1(benchmark::State& state) {
	std::vector<std::string> v1 = { "v1a", "v1b", "v1c"};
	std::vector<std::string> v2 = { "v2a", "v2b"};
	std::vector<std::string> v3 = { "v3a", "v3b", "v3c"};
	std::vector<std::string> v4 = { "v4a"};
	std::vector<std::string> v5 = { "v5a", "v5b", "v5c", "v5d", "v5e", "v5f"};
	std::vector<std::string> v6 = { "v6a", "v6b", "v6c"};
	std::vector<const std::vector<std::string>*> vecs = { &v1, &v2, &v3, &v4, &v5, &v6 };

	for (auto _ : state) {
		auto product = Product(vecs);
		benchmark::DoNotOptimize(product);
		benchmark::ClobberMemory();
	}
	// auto x = Product(vec);
	// std::cout << x;
}
BENCHMARK(Instance1)->Unit(benchmark::kMillisecond);
// 0.055 ms