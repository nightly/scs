#include <benchmark/benchmark.h>

#include "scs/FirstOrderLogic/fol.h"

static void Equality(benchmark::State& state) {
	using namespace scs;
	scs::Situation s;
	s.objects.emplace("robot1");
	s.objects.emplace("robot2");
	scs::Evaluator eval{ s };

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