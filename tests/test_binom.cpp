#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "libvol/models/binom.hpp"
#include "libvol/models/black_scholes.hpp"
#include <cmath>

TEST_CASE("Binomial pricing: no-dividend American call equal to euro", "[binomial]"){
    double S = 100.0;
    double K = 100.0;
    double r = 0.05;
    double q = 0.0;
    double T = 1.0;
    double vol = 0.25;
    int steps = 100;
    bool is_call = true;

    double american_call = vol::binom::price(S, K, r, q, T, vol, steps, is_call, /*is_american=*/true);
    double euro_call     = vol::bs::price  (S, K, r, q, T, vol, is_call);

    REQUIRE_THAT(american_call, Catch::Matchers::WithinRel(euro_call, 0.005)); // 0.5% tolerance
}

TEST_CASE("Binomial pricing: American put has early exercise premium", "[binomial]"){
    double S = 100.0;
    double K = 100.0;
    double r = 0.05;
    double q = 0.0;
    double T = 1.0;
    double vol = 0.25;
    int steps = 100;
    bool is_call = false;
    bool is_american = true;

    // FIXED PARAM ORDER
    double american_put = vol::binom::price(S, K, r, q, T, vol, steps, is_call, is_american);
    double euro_put     = vol::bs::price  (S, K, r, q, T, vol, is_call);

    REQUIRE(american_put > euro_put);
    REQUIRE(american_put < euro_put * 1.5); // sanity bound
}

TEST_CASE("Binomial pricing: Binomial euro call converges to Black_Scholes", "[binomial]"){
    double S = 100.0;
    double K = 100.0;
    double r = 0.05;
    double q = 0.02;
    double T = 1.0;
    double vol = 0.25;
    bool is_call = true;
    bool is_american = false;

    // FIXED PARAM ORDER
    double euro_call_binom_50  = vol::binom::price(S, K, r, q, T, vol,  50, is_call, is_american);
    double euro_call_binom_100 = vol::binom::price(S, K, r, q, T, vol, 100, is_call, is_american);
    double euro_call_binom_200 = vol::binom::price(S, K, r, q, T, vol, 200, is_call, is_american);
    double euro_call_bs        = vol::bs::price  (S, K, r, q, T, vol, is_call);

    double err_50  = euro_call_binom_50  - euro_call_bs;
    double err_100 = euro_call_binom_100 - euro_call_bs;
    double err_200 = euro_call_binom_200 - euro_call_bs;

    // Compare absolute errors; binomial error sign can flip.
    REQUIRE(std::abs(err_100) <= std::abs(err_50));
    REQUIRE(std::abs(err_200) <= std::abs(err_100));
    REQUIRE(std::abs(err_200) < 0.1);
}

TEST_CASE("Binomial pricing: Euro put-call parity", "[binomial]"){
    double S = 100.0;
    double K = 100.0;
    double r = 0.05;
    double q = 0.02;
    double T = 1.0;
    double vol = 0.25;
    int steps = 100;
    bool is_american = false;

    // FIXED PARAM ORDER
    double call = vol::binom::price(S, K, r, q, T, vol, steps, /*is_call=*/true,  is_american);
    double put  = vol::binom::price(S, K, r, q, T, vol, steps, /*is_call=*/false, is_american);

    double lhs = call - put;
    // Put-call parity with continuous dividend yield q:
    double rhs = S * std::exp(-q * T) - K * std::exp(-r * T);

    REQUIRE_THAT(lhs, Catch::Matchers::WithinRel(rhs, 0.01)); // 1% relative tolerance
}
