#include <benchmark/benchmark.h>

#include "demo/vec.hpp"
#include "throwers/throws.hpp"

static void BM_CreateInt(benchmark::State& state) { // NOLINT(runtime/references)
  for (auto _ : state) {
    auto* p = throws::make_int();
    benchmark::DoNotOptimize(p);
    delete p;
  }
}
BENCHMARK(BM_CreateInt);

static void BM_CreateDouble(benchmark::State& state) { // NOLINT(runtime/references)
  for (auto _ : state) {
    auto* p = throws::make_double();
    benchmark::DoNotOptimize(p);
    delete p;
  }
}
BENCHMARK(BM_CreateDouble);

BENCHMARK_MAIN();
