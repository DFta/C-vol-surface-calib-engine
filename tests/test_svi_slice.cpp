#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
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
    std::vector<double> log_moneyness;
};

SliceMarketData make_slice(const vol::svi::Params& params, double S, double r, double q, double T) {
    SliceMarketData data;
    const double F = S * std::exp((r - q) * T);
    const std::vector<double> k_grid = { -0.80, -0.60, -0.40, -0.20, -0.10, 0.0, 0.10, 0.20, 0.40, 0.60, 0.80 };
    data.options.reserve(k_grid.size());
    data.mids.reserve(k_grid.size());
    data.log_moneyness = k_grid;

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
}

TEST_CASE("SVI slice calibration recovers synthetic smile", "[svi][slice]") {
    const vol::svi::Params truth { 0.035, 0.18, -0.35, -0.05, 0.22 };
    const auto market = make_slice(truth, 100.0, 0.01, 0.0, 0.75);

    const auto cfg = vol::svi::SliceConfig{};
    const auto params = vol::svi::calibrate_slice_from_prices(market.options, market.mids, cfg);

    REQUIRE(vol::svi::basic_no_arb(params));

    for (double k : market.log_moneyness) {
        const double w_truth = vol::svi::total_variance(k, truth);
        const double w_fit = vol::svi::total_variance(k, params);
        REQUIRE_THAT(w_fit, Catch::Matchers::WithinRel(w_truth, 0.1));
    }
}

TEST_CASE("SVI slice calibration skips invalid quotes", "[svi][slice]") {
    const vol::svi::Params truth { 0.030, 0.16, -0.25, -0.02, 0.21 };
    auto market = make_slice(truth, 90.0, 0.005, 0.0, 0.5);

    // Knock out a few points to exercise skipping logic.
    REQUIRE(market.mids.size() >= 6);
    market.mids[1] = 0.0;
    market.mids[4] = 0.0;
    market.mids.back() = 0.0;

    vol::svi::SliceConfig cfg;
    cfg.min_points = 4;

    const auto params = vol::svi::calibrate_slice_from_prices(market.options, market.mids, cfg);
    REQUIRE(vol::svi::basic_no_arb(params));

    for (double k : market.log_moneyness) {
        const double w_truth = vol::svi::total_variance(k, truth);
        const double w_fit = vol::svi::total_variance(k, params);
        REQUIRE(std::abs(w_fit - w_truth) < 0.05);
    }
}

TEST_CASE("SVI slice calibration falls back when no data", "[svi][slice]") {
    std::vector<OptionSpec> empty_opts(5, OptionSpec{100.0, 100.0, 0.01, 0.0, 1.0, true});
    std::vector<double> zero_mids(5, 0.0);

    const auto cfg = vol::svi::SliceConfig{};
    const auto params = vol::svi::calibrate_slice_from_prices(empty_opts, zero_mids, cfg);

    REQUIRE(params[0] >= 0.0);
    REQUIRE(params[1] > 0.0);
    REQUIRE(params[4] > 0.0);
}