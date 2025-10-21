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
double S=100,K=100,r=0.02,q=0.01,T=1,vol=0.25; double h=1e-5;
auto g = vol::bs::price_greeks(S,K,r,q,T,vol,true);
double f0 = g.price;
double fS1 = vol::bs::price(S+h,K,r,q,T,vol,true);
double fS2 = vol::bs::price(S-h,K,r,q,T,vol,true);
double num_delta = (fS1 - fS2)/(2*h);
double num_gamma = (fS1 - 2*f0 + fS2)/(h*h);
REQUIRE(std::abs(g.delta - num_delta) < 1e-5);
REQUIRE(std::abs(g.gamma - num_gamma) < 1e-4);
}