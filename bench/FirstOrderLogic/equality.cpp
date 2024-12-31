#include <benchmark/benchmark.h>

#include "scs/FirstOrderLogic/fol.h"
#include "memory_manager.h"

static void Equality(benchmark::State& state) {
	using namespace scs;
	scs::Situation s;
	scs::BasicActionTheory bat;
	bat.objects.emplace("robot1");
	bat.objects.emplace("robot2");
	scs::Evaluator eval{ {s, bat} };

	for (auto _ : state) {
		scs::Formula f = BinaryConnective(scs::Object{ "robot1" }, scs::Object{ "robot1" }, BinaryKind::Equal);
		auto result = std::visit(eval, f);

		f = BinaryConnective(scs::Object{ "robot1" }, scs::Object{ "robot2" }, BinaryKind::Equal);
		result = std::visit(eval, f);

		f = Quantifier("x", BinaryConnective(scs::Object{ "robot1" }, scs::Variable{ "x" }, BinaryKind::Equal), QuantifierKind::Existential);
		result = std::visit(eval, f);

		benchmark::DoNotOptimize(result);
		benchmark::ClobberMemory();
	}
}

// BENCHMARK(Equality)->Unit(benchmark::kMillisecond);
BENCHMARK(Equality);
// 588-1000ns, 385ns CPU

//BENCHMARK_MAIN();
int main(int argc, char** argv)
{
	::benchmark::RegisterMemoryManager(mm.get());
	::benchmark::Initialize(&argc, argv);
	::benchmark::RunSpecifiedBenchmarks();
	::benchmark::RegisterMemoryManager(nullptr);
}