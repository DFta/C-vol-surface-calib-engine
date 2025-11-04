#pragma once
#include "libvol/core/types.hpp"
#include "libvol/models/svi.hpp"
#include <vector>

namespace vol::svi {

struct SliceConfig {
    bool   use_vega_weights = true;
    double wing_dampen_pow  = 2.0;   // downweight far wings: w *= 1 / (1 + |k|^p)
    double min_vega_eps     = 1e-8;  // floor to avoid zeros
    int    min_points       = 6;     // need at least this many valid points
};

// Calibrate a single-maturity SVI slice from market prices.
// Returns raw SVI params {a,b,rho,m,sigma}. Skips points with failed IV.
Params calibrate_slice_from_prices(const std::vector<OptionSpec>& opts, const std::vector<double>& mids,const SliceConfig& cfg = {});

} // namespace vol::svi
