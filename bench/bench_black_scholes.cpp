#include <benchmark/benchmark.h>
#include "libvol/models/black_scholes.hpp"

// Benchmark single price calculation (typical ATM option)
static void BM_Price_ATM(benchmark::State& state) {
    for (auto _ : state) {
        double result = vol::bs::price(100.0, 100.0, 0.05, 0.02, 1.0, 0.25, true);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Price_ATM);

// Benchmark single price calculation (OTM option)
static void BM_Price_OTM(benchmark::State& state) {
    for (auto _ : state) {
        double result = vol::bs::price(100.0, 110.0, 0.05, 0.02, 1.0, 0.25, true);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Price_OTM);

// Benchmark Greeks calculation (more realistic use case)
static void BM_PriceGreeks_ATM(benchmark::State& state) {
    for (auto _ : state) {
        auto result = vol::bs::price_greeks(100.0, 100.0, 0.05, 0.02, 1.0, 0.25, true);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_PriceGreeks_ATM);

// Benchmark varying strikes (portfolio pricing scenario)
static void BM_Price_MultipleStrikes(benchmark::State& state) {
    const int n_strikes = 10;
    double strikes[n_strikes];
    for (int i = 0; i < n_strikes; ++i) {
        strikes[i] = 90.0 + i * 2.0;  // Strikes from 90 to 108
    }
    
    for (auto _ : state) {
        double total = 0.0;
        for (int i = 0; i < n_strikes; ++i) {
            total += vol::bs::price(100.0, strikes[i], 0.05, 0.02, 1.0, 0.25, true);
        }
        benchmark::DoNotOptimize(total);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_Price_MultipleStrikes);

// Benchmark short-dated options (potential edge case)
static void BM_Price_ShortDated(benchmark::State& state) {
    for (auto _ : state) {
        double result = vol::bs::price(100.0, 100.0, 0.05, 0.02, 0.01, 0.25, true);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Price_ShortDated);

// Benchmark put options
static void BM_Price_Put(benchmark::State& state) {
    for (auto _ : state) {
        double result = vol::bs::price(100.0, 100.0, 0.05, 0.02, 1.0, 0.25, false);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Price_Put);

BENCHMARK_MAIN();