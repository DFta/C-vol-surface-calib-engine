#include "libvol/calib/svi_slice.hpp"
#include <iostream>
#include <vector>

int main() {
    using vol::OptionSpec;
    using vol::svi::SliceConfig;
    using vol::svi::Params;

    //Example, replace with real quotes
    const double S=50, r=0.02, q=0.02, T=0.5;
    std::vector<OptionSpec> opts = {
        {S, 80, r, q, T, true}, {S,  90, r, q, T, true}, {S, 100, r, q, T, true},
        {S, 110, r, q, T, true}, {S, 120, r, q, T, true}
    };
    std::vector<double> mids = {21.2, 12.7, 6.8, 3.1, 1.2}; // replace with market mids

    SliceConfig cfg;
    Params theta = vol::svi::calibrate_slice_from_prices(opts, mids, cfg);

    double F = S * std::exp((r - q) * T);
    double K = 105.0;
    double k = std::log(K / F);
    double w = vol::svi::total_variance(k, theta);
    double iv = std::sqrt(std::max(0.0, w / T));

    std::cout << "SVI params {a,b,rho,m,sigma} = {"
            << theta[0] << ", " << theta[1] << ", " << theta[2] << ", "
            << theta[3] << ", " << theta[4] << "}\n";
    std::cout << "IV at K=105,T=0.5: " << iv << "\n";
    return 0;
}
