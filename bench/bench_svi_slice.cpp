#include <benchmark/benchmark.h>
#include "libvol/calib/svi_slice.hpp"
#include "libvol/models/black_scholes.hpp"
#include "libvol/models/svi.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

using vol::OptionSpec;

namespace {
struct SliceMarketData {
    std::vector<OptionSpec> options;
    std::vector<double> mids;
};

SliceMarketData make_slice(const vol::svi::Params& params, double S, double r, double q, double T) {
    SliceMarketData data;
    const double F = S * std::exp((r - q) * T);
    const std::vector<double> k_grid = { -0.80, -0.60, -0.40, -0.20, -0.10, 0.0, 0.10, 0.20, 0.40, 0.60, 0.80 };
    data.options.reserve(k_grid.size());
    data.mids.reserve(k_grid.size());

    for (double k : k_grid) {
        const double K = F * std::exp(k);
        const double w = vol::svi::total_variance(k, params);
        const double iv = std::sqrt(std::max(1e-10, w / T));
        const double price = vol::bs::price(S, K, r, q, T, iv, /*is_call=*/true);

        data.options.push_back(OptionSpec{S, K, r, q, T, true});
        data.mids.push_back(price);
    }

    return data;
}

SliceMarketData make_noisy_slice(const SliceMarketData& base) {
    SliceMarketData noisy = base;
    const std::vector<double> distortions = { 0.0, 0.01, -0.015, 0.02, -0.01, 0.0, 0.015, -0.02, 0.01, -0.015, 0.0 };
    const double scale = 0.02; // up to 2% noise
    for (std::size_t i = 0; i < noisy.mids.size() && i < distortions.size(); ++i) {
        noisy.mids[i] *= (1.0 + scale * distortions[i]);
        noisy.mids[i] = std::max(1e-8, noisy.mids[i]);
    }
    return noisy;
}

const vol::svi::Params k_base_slice { 0.035, 0.18, -0.35, -0.05, 0.22 };
const SliceMarketData k_clean_slice = make_slice(k_base_slice, 100.0, 0.01, 0.0, 0.75);
const SliceMarketData k_noisy_slice = make_noisy_slice(k_clean_slice);

std::vector<SliceMarketData> make_term_structure() {
    const std::vector<vol::svi::Params> term_params = {
        { 0.025, 0.15, -0.30, -0.04, 0.20 },
        { 0.030, 0.18, -0.25, -0.03, 0.24 },
        { 0.040, 0.22, -0.20, -0.02, 0.28 },
        { 0.055, 0.26, -0.10, -0.01, 0.32 }
    };

    std::vector<SliceMarketData> slices;
    slices.reserve(term_params.size());

    const double S = 120.0;
    const double r = 0.015;
    const double q = 0.005;
    const std::vector<double> tenors = { 0.25, 0.5, 1.0, 2.0 };

    for (std::size_t i = 0; i < term_params.size(); ++i) {
        slices.push_back(make_slice(term_params[i], S, r, q, tenors[i]));
    }
    return slices;
}

const std::vector<SliceMarketData> k_term_slices = make_term_structure();

} // namespace

static void BM_SVI_Calibrate_Clean(benchmark::State& state) {
    const auto cfg = vol::svi::SliceConfig{};
    for (auto _ : state) {
        auto params = vol::svi::calibrate_slice_from_prices(k_clean_slice.options, k_clean_slice.mids, cfg);
        benchmark::DoNotOptimize(params);
    }
}
BENCHMARK(BM_SVI_Calibrate_Clean);

static void BM_SVI_Calibrate_Noisy(benchmark::State& state) {
    auto cfg = vol::svi::SliceConfig{};
    cfg.use_vega_weights = false; // stress alternate weighting
    for (auto _ : state) {
        auto params = vol::svi::calibrate_slice_from_prices(k_noisy_slice.options, k_noisy_slice.mids, cfg);
        benchmark::DoNotOptimize(params);
    }
}
BENCHMARK(BM_SVI_Calibrate_Noisy);

static void BM_SVI_Calibrate_TermStructure(benchmark::State& state) {
    const auto cfg = vol::svi::SliceConfig{};
    for (auto _ : state) {
        vol::svi::Params accum = {};
        for (const auto& slice : k_term_slices) {
            auto params = vol::svi::calibrate_slice_from_prices(slice.options, slice.mids, cfg);
            benchmark::DoNotOptimize(params);
            for (std::size_t i = 0; i < accum.size(); ++i) {
                accum[i] += params[i];
            }
        }
        benchmark::DoNotOptimize(accum);
    }
}
BENCHMARK(BM_SVI_Calibrate_TermStructure);

BENCHMARK_MAIN();