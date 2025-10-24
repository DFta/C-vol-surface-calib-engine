#include <benchmark/benchmark.h>
#include "libvol/models/binom.hpp"
#include "libvol/models/black_scholes.hpp"

// --- Helpers / default params ---
struct Params {
    double S=100, K=100, r=0.05, q=0.02, T=1.0, vol=0.25;
};

// -------- Single price (European call), sweep steps ----------
static void BM_Binom_Price_Euro_Call(benchmark::State& state) {
    Params p;
    const bool is_call = true, is_american = false;
    const int steps = static_cast<int>(state.range(0)); // sweep N
    for (auto _ : state) {
        double px = vol::binom::price(p.S, p.K, p.r, p.q, p.T, p.vol, steps, is_call, is_american);
        benchmark::DoNotOptimize(px);
    }
    state.counters["steps"] = steps;
}
BENCHMARK(BM_Binom_Price_Euro_Call)->RangeMultiplier(2)->Range(50, 4096);

// -------- American put (stress early exercise), sweep steps ----------
static void BM_Binom_Price_Amer_Put(benchmark::State& state) {
    Params p; p.q = 0.0; // classic early-ex case
    const bool is_call = false, is_american = true;
    const int steps = static_cast<int>(state.range(0));
    for (auto _ : state) {
        double px = vol::binom::price(p.S, p.K, p.r, p.q, p.T, p.vol, steps, is_call, is_american);
        benchmark::DoNotOptimize(px);
    }
    state.counters["steps"] = steps;
}
BENCHMARK(BM_Binom_Price_Amer_Put)->RangeMultiplier(2)->Range(50, 4096);

// -------- Greeks (finite-diff), European call, sweep steps ----------
static void BM_Binom_Greeks_Euro_Call(benchmark::State& state) {
    Params p;
    const bool is_call = true, is_american = false;
    const int steps = static_cast<int>(state.range(0));
    for (auto _ : state) {
        auto g = vol::binom::price_greeks(p.S, p.K, p.r, p.q, p.T, p.vol, steps, is_call, is_american);
        benchmark::DoNotOptimize(g);
    }
    state.counters["steps"] = steps;
}
BENCHMARK(BM_Binom_Greeks_Euro_Call)->RangeMultiplier(2)->Range(50, 1024);

// -------- Portfolio-ish: multiple strikes in a loop ----------
static void BM_Binom_Price_MultipleStrikes(benchmark::State& state) {
    Params p;
    const bool is_call = true, is_american = false;
    const int steps = static_cast<int>(state.range(0));
    const int n = 16;
    double strikes[n];
    for (int i = 0; i < n; ++i) strikes[i] = 80.0 + i * 5.0;

    for (auto _ : state) {
        double total = 0.0;
        for (int i = 0; i < n; ++i) {
            total += vol::binom::price(p.S, strikes[i], p.r, p.q, p.T, p.vol, steps, is_call, is_american);
        }
        benchmark::DoNotOptimize(total);
        benchmark::ClobberMemory();
    }
    state.counters["steps"] = steps;
    state.counters["batch"] = n;
}
BENCHMARK(BM_Binom_Price_MultipleStrikes)->RangeMultiplier(2)->Range(50, 1024);

// -------- Short-dated (edge-case), European call ----------
static void BM_Binom_Price_ShortDated(benchmark::State& state) {
    Params p; p.T = 0.02; // ~1 week
    const bool is_call = true, is_american = false;
    const int steps = static_cast<int>(state.range(0));
    for (auto _ : state) {
        double px = vol::binom::price(p.S, p.K, p.r, p.q, p.T, p.vol, steps, is_call, is_american);
        benchmark::DoNotOptimize(px);
    }
    state.counters["steps"] = steps;
}
BENCHMARK(BM_Binom_Price_ShortDated)->RangeMultiplier(2)->Range(50, 2048);

// -------- Cross-check: binomial vs BS in the tight loop (optional sanity) --------
// Not strictly a performance test—just ensures we aren't wildly off while sweeping N.
static void BM_Binom_Error_vs_BS(benchmark::State& state) {
    Params p;
    const bool is_call = true, is_american = false;
    const int steps = static_cast<int>(state.range(0));
    const double bs = vol::bs::price(p.S, p.K, p.r, p.q, p.T, p.vol, is_call);

    for (auto _ : state) {
        double b = vol::binom::price(p.S, p.K, p.r, p.q, p.T, p.vol, steps, is_call, is_american);
        benchmark::DoNotOptimize(b);
        benchmark::DoNotOptimize(bs);
    }
    // Expose the last-seen absolute error as a counter (not used for control flow)
    const double b_last = vol::binom::price(p.S, p.K, p.r, p.q, p.T, p.vol, steps, is_call, is_american);
    state.counters["abs_err_vs_BS"] = std::abs(b_last - vol::bs::price(p.S, p.K, p.r, p.q, p.T, p.vol, is_call));
    state.counters["steps"] = steps;
}
BENCHMARK(BM_Binom_Error_vs_BS)->RangeMultiplier(2)->Range(50, 1024);

BENCHMARK_MAIN();
