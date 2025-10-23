#pragma once
#include <array>
#include <vector>


namespace vol::svi {
// Raw SVI: w(k) = a + b * ( rho*(k-m) + sqrt((k-m)^2 + sigma^2) ) where w is total variance
using Params = std::array<double,5>; // {a,b,rho,m,sigma}


double total_variance(double k, const Params& p);


// Simple no-arb sanity checks (not exhaustive): b>0, |rho|<1, sigma>0
bool basic_no_arb(const Params& p);


// Fit a single expiry smile: minimize squared error between market total variance and SVI
// Inputs: log-moneyness k[i], market total variance w_mkt[i], weights
Params fit_raw_svi(const std::vector<double>& k, const std::vector<double>& w_mkt,
const std::vector<double>& wts);
}