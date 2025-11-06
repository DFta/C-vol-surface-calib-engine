#include "libvol/calib/svi_slice.hpp"
#include "libvol/models/black_scholes.hpp"
#include <cmath>
#include <algorithm>

namespace vol::svi {

static inline double clamp(double x, double lo, double hi) {
    return std::max(lo, std::min(hi, x));
}

Params calibrate_slice_from_prices(const std::vector<OptionSpec>& opts, const std::vector<double>& mids, const SliceConfig& cfg)
{
    const std::size_t n = std::min(opts.size(), mids.size());
    if (n == 0) {
        return Params{1e-8, 0.1, 0.0, 0.0, 0.2};
    }

    // slice should be single maturity, if not something didn't work in compiling
    const double T0 = opts[0].T;
    const double tolT = 1e-10;
    for (std::size_t i = 1; i < n; ++i) {
        if (std::abs(opts[i].T - T0) > tolT) {
            break;
        }
    }

    std::vector<double> kvals;      kvals.reserve(n);
    std::vector<double> w_market;   w_market.reserve(n);
    std::vector<double> weights;    weights.reserve(n);

    for (std::size_t i = 0; i < n; ++i) {
        const auto& o = opts[i];
        const double mid = mids[i];
        if (mid <= 0.0) continue;

        // IV from price
        auto ivr = bs::implied_vol(o.S, o.K, o.r, o.q, o.T, mid, o.is_call, 0.2, 1e-10);
        if (!ivr.converged || !std::isfinite(ivr.iv) || ivr.iv <= 0.0) continue;

        const double F  = o.S * std::exp((o.r - o.q) * o.T);
        const double k  = std::log(o.K / F);
        const double w  = ivr.iv * ivr.iv * o.T;

        double wt = 1.0;
        if (cfg.use_vega_weights) {
            auto g = bs::price_greeks(o.S, o.K, o.r, o.q, o.T, ivr.iv, o.is_call);
            wt = std::max(cfg.min_vega_eps, g.vega);
        }
        // dampen far-wings a bit (helps stability)
        wt *= 1.0 / (1.0 + std::pow(std::abs(k), cfg.wing_dampen_pow));

        kvals.push_back(k);
        w_market.push_back(w);
        weights.push_back(wt);
    }

    if (kvals.size() < static_cast<std::size_t>(std::max(3, cfg.min_points))) {
        // fallback symmetric, low-curvature
        const double kmin = (kvals.empty() ? -0.1 : *std::min_element(kvals.begin(), kvals.end()));
        const double kmax = (kvals.empty() ?  0.1 : *std::max_element(kvals.begin(), kvals.end()));
        return Params{1e-8, 0.1, 0.0, 0.5 * (kmin + kmax), 0.2};
    }

    return fit_raw_svi(kvals, w_market, weights);
}

} // namespace vol::svi
