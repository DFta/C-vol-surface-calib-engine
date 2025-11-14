#include <benchmark/benchmark.h>

#include <string>

#include "libvol/models/heston.hpp"

namespace {

constexpr vol::heston::Params ATM_PARAMS{1.5, 0.04, 0.5, -0.7, 0.04};
constexpr vol::heston::Params STRESSED_PARAMS{2.5, 0.09, 0.7, -0.5, 0.08};

} // namespace

static void BM_Heston_ATM_Call64(benchmark::State& state) {
    for (auto _ : state) {
        const double price = vol::heston::price_cf(
            100.0, 100.0, 0.01, 0.0, 1.0, ATM_PARAMS, true, 64);
        benchmark::DoNotOptimize(price);
    }
}
BENCHMARK(BM_Heston_ATM_Call64);

static void BM_Heston_OTM_Call64(benchmark::State& state) {
    for (auto _ : state) {
        const double price = vol::heston::price_cf(
            100.0, 120.0, 0.01, 0.0, 1.0, ATM_PARAMS, true, 64);
        benchmark::DoNotOptimize(price);
    }
}
BENCHMARK(BM_Heston_OTM_Call64);

static void BM_Heston_ATM_Put64(benchmark::State& state) {
    for (auto _ : state) {
        const double price = vol::heston::price_cf(
            100.0, 100.0, 0.01, 0.0, 1.0, ATM_PARAMS, false, 64);
        benchmark::DoNotOptimize(price);
    }
}
BENCHMARK(BM_Heston_ATM_Put64);

static void BM_Heston_Portfolio64(benchmark::State& state) {
    constexpr int N = 8;
    const double strikes[N] = {70, 80, 90, 100, 110, 120, 140, 160};
    for (auto _ : state) {
        double total = 0.0;
        for (double K : strikes) {
            total += vol::heston::price_cf(
                100.0, K, 0.015, 0.0, 2.0, STRESSED_PARAMS, true, 64);
        }
        benchmark::DoNotOptimize(total);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_Heston_Portfolio64);

static void BM_Heston_GaussLaguerreOrder(benchmark::State& state) {
    const int n_gl = static_cast<int>(state.range(0));
    for (auto _ : state) {
        const double price = vol::heston::price_cf(
            100.0, 90.0, 0.01, 0.0, 0.5, STRESSED_PARAMS, true, n_gl);
        benchmark::DoNotOptimize(price);
    }
    state.SetLabel("n_gl=" + std::to_string(n_gl));
}
BENCHMARK(BM_Heston_GaussLaguerreOrder)->Arg(32)->Arg(64)->Arg(96);

BENCHMARK_MAIN();
