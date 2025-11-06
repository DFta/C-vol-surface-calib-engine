#include <catch2/catch_all.hpp>
#include "libvol/models/black_scholes.hpp"
#include <cmath>


TEST_CASE("BS price symmetry put-call parity", "[bs]"){
double S=100,K=100,r=0.02,q=0.01,T=1,vol=0.2;
double c = vol::bs::price(S,K,r,q,T,vol,true);
double p = vol::bs::price(S,K,r,q,T,vol,false);
double parity = c - p - (S*std::exp(-q*T) - K*std::exp(-r*T));
REQUIRE(std::abs(parity) < 1e-10);
}


TEST_CASE("BS greeks vs finite diff", "[bs]"){
    double S=100, K=100, r=0.02, q=0.01, T=1, vol=0.25;

    double h = std::max(1e-4 * S, 1e-6);

    auto g = vol::bs::price_greeks(S, K, r, q, T, vol, true);

    // Delta from price (central diff)
    double f_up = vol::bs::price(S+h, K, r, q, T, vol, true);
    double f_dn = vol::bs::price(S-h, K, r, q, T, vol, true);
    double num_delta = (f_up - f_dn) / (2*h);

    // Gamma as derivative of delta (central diff on delta)
    auto g_up = vol::bs::price_greeks(S+h, K, r, q, T, vol, true);
    auto g_dn = vol::bs::price_greeks(S-h, K, r, q, T, vol, true);
    double num_gamma = (g_up.delta - g_dn.delta) / (2*h);

    INFO("delta analytic=" << g.delta << " numeric=" << num_delta);
    INFO("gamma analytic=" << g.gamma << " numeric=" << num_gamma);

    REQUIRE(std::abs(g.delta - num_delta) < 1e-5);
    REQUIRE(std::abs(g.gamma - num_gamma) < 1e-4);
}

TEST_CASE("BS put theta vs finite diff", "[bs]") {
    double S=100, K=100, r=0.02, q=0.01, T=1.0, vol=0.25;
    bool is_call = false;

    auto g = vol::bs::price_greeks(S, K, r, q, T, vol, is_call);

    double hT = 1e-4;
    double p_up = vol::bs::price(S, K, r, q, T + hT, vol, is_call);
    double p_dn = vol::bs::price(S, K, r, q, T - hT, vol, is_call);
    double dV_dT = (p_up - p_dn) / (2.0 * hT);
    double num_theta = -1.0 * dV_dT;

    REQUIRE(std::abs(g.theta - num_theta) < 1e-4);
}

