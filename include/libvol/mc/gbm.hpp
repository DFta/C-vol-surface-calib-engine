#pragma once
#include <cstdint>

namespace vol::mc {

struct MCResult {
    double price;
    double std_err;      
    std::uint64_t paths;
};

// Antithetic + control variate (BS control)
MCResult european_vanilla_gbm(double S,double K,double r,double q,double T,double vol,bool is_call,
                                std::uint64_t n_paths, std::uint64_t seed=42);

} // namespace vol::mc
