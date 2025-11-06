#pragma once
#include "libvol/core/types.hpp"
#include "libvol/models/svi.hpp"
#include <vector>

namespace vol::svi {

struct SliceConfig {
    bool   use_vega_weights = true;
    double wing_dampen_pow  = 2.0;  
    double min_vega_eps     = 1e-8;  // floor to avoid zeros
    int    min_points       = 6;    
};

Params calibrate_slice_from_prices(const std::vector<OptionSpec>& opts, const std::vector<double>& mids,const SliceConfig& cfg = {});

} // namespace vol::svi
