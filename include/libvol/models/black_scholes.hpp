#pragma once
#include <tuple>


namespace vol::bs {
struct PriceGreeks { double price, delta, gamma, vega, theta, rho; };


// Standard normal pdf/cdf
double phi(double x);
double Phi(double x);


// Black-Scholes with continuous dividend yield q
double price(double S,double K,double r,double q,double T,double vol,bool is_call);
PriceGreeks price_greeks(double S,double K,double r,double q,double T,double vol,bool is_call);


// Robust implied vol: Newton with Brent fallback and safe-guards
struct IVResult { double iv; int newton_iters; int brent_iters; bool converged; };
IVResult implied_vol(double S,double K,double r,double q,double T,double price,bool is_call,
double init=0.2, double tol=1e-10);
}