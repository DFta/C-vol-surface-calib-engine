#pragma once
#include <cmath>
#include <utility>
#include <vector>
#include <cstddef>

namespace vol::stats {
inline std::pair<double,double> mean_var(const std::vector<double>& x){
    const std::size_t n = x.size();
    if (n == 0) return {0.0, 0.0};
    double m = 0.0, m2 = 0.0; std::size_t k = 0;
    for (double v : x) { ++k; double d = v - m; m += d / k; m2 += d * (v - m); }
    return { m, (n > 1 ? m2/(n-1) : 0.0) };
}
inline double std_err(const std::vector<double>& x){  // was: stderr
    auto mv = mean_var(x);
    const double v = mv.second;
    return x.empty() ? 0.0 : std::sqrt(v/x.size());
}
} // namespace vol::stats
