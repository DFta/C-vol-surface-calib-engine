#include <catch2/catch_all.hpp>
#include "libvol/mc/gbm.hpp"
#include "libvol/models/black_scholes.hpp"
#include <cmath>

TEST_CASE("Monte-Carlo vs Black-Scholes call, 200k paths","[gbm]"){
    double S=100,K=100,r=0.02,q=0.01,T=1,vol=0.2;
    bool is_call = true;
    auto res_mc = vol::mc::european_vanilla_gbm(S,K,r,q,T,vol,is_call,2e5);
    double price_mc = res_mc.price;
    double se_mc = res_mc.std_err;
    double price_bs = vol::bs::price(S,K,r,q,T,vol,is_call);

    double diff = std::abs(price_mc - price_bs);

    REQUIRE(diff < 4.0 * se_mc); //diff within 4 stderr
}

TEST_CASE("Monte-Carlo vs Black-Scholes put, 200k paths","[gbm]"){
    double S=100,K=100,r=0.02,q=0.01,T=1,vol=0.2;
    bool is_call = false;
    auto res_mc = vol::mc::european_vanilla_gbm(S,K,r,q,T,vol,is_call,2e5);
    double price_mc = res_mc.price;
    double se_mc = res_mc.std_err;
    double price_bs = vol::bs::price(S,K,r,q,T,vol,is_call);

    double diff = std::abs(price_mc - price_bs);

    REQUIRE(diff < 4.0 * se_mc); //same as above
}
