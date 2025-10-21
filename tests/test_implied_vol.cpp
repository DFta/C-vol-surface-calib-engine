#include <catch2/catch_all.hpp>
#include "libvol/models/black_scholes.hpp"


TEST_CASE("IV recovers input vol", "[iv]"){
double S=100,K=120,r=0.01,q=0.00,T=0.75,vol=0.35; bool call=false;
double px = vol::bs::price(S,K,r,q,T,vol,call);
auto ivr = vol::bs::implied_vol(S,K,r,q,T,px,call,0.2,1e-12);
REQUIRE(ivr.converged);
REQUIRE(std::abs(ivr.iv - vol) < 1e-8);
}