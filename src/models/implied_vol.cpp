#include "libvol/models/black_scholes.hpp"
#include "libvol/math/root_finders.hpp"
#include "libvol/core/constants.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace vol::bs {

    [[nodiscard]] IVResult implied_vol(double S, double K, double r, double q,double T, double target, bool is_call,double init, double tol){
        using namespace vol::root;

        // basic input validation
        if (!(std::isfinite(S) && std::isfinite(K) && std::isfinite(r) && std::isfinite(q) && std::isfinite(T) && std::isfinite(target))) {
            return {std::numeric_limits<double>::quiet_NaN(), 0, 0, false};
        }
        if (S <= 0.0 || K <= 0.0 || T <= 0.0 || target < 0.0) {
            return {std::numeric_limits<double>::quiet_NaN(), 0, 0, false};
        }

        // useful constants
        constexpr double MIN_SIGMA = 1e-9;     
        constexpr double MAX_SIGMA = 5.0;      
        constexpr int MAX_NEWTON_ITERS = 20;
        constexpr int MAX_BRENT_ITERS  = 100;

        const double df_r = std::exp(-r * T);
        const double df_q = std::exp(-q * T);
        const double F = S * std::exp((r - q) * T);

        const double intrinsic = (is_call
            ? std::max(0.0, S * df_q - K * df_r)
            : std::max(0.0, K * df_r - S * df_q));

        const double max_price = is_call ? S * df_q : K * df_r;

        const double rel_eps = 1e-10;

        //bounds for impossible values
        if (target < intrinsic * (1.0 - rel_eps)) {
            return {std::numeric_limits<double>::quiet_NaN(), 0, 0, false};
        }
        if (target <= intrinsic * (1.0 + 1e-4)) {
            return {0.0, 0, 0, true};
        }
        if (target > max_price * (1.0 + rel_eps)) {
            return {std::numeric_limits<double>::quiet_NaN(), 0, 0, false};
        }
        if (target >= max_price * (1.0 - 1e-12)) {
            return {std::numeric_limits<double>::quiet_NaN(), 0, 0, false};
        }

        if (T < 1.0 / 365.0) {
            tol = std::max(tol, 1e-6);
        }
        const double vol_tol = std::max(1e-12, tol);
        const double price_tol = std::max(1e-12, 1e-8 * std::max(1.0, target));

        auto f_price = [&](double sigma) -> double {
            return price(S, K, r, q, T, sigma, is_call) - target;
        };

        auto f_price_vega = [&](double sigma, double& f, double& vega) {
            auto g = price_greeks(S, K, r, q, T, sigma, is_call);
            f = g.price - target;
            vega = g.vega;// per 1.0 vol (not %)
        };

        //initial guess
        double sigma = std::numeric_limits<double>::quiet_NaN();
        const bool init_ok = (init > 0.0 && init < MAX_SIGMA && std::isfinite(init));

        if (init_ok) {
            sigma = init;
        } else {
            const double c_norm = target / (S * df_q);
            const double guess = std::sqrt(std::max(1e-12, (2.0 * vol::PI / T))) * std::max(1e-12, c_norm);
            const double xm = std::abs(std::log((F + 1e-300) / (K + 1e-300)));
            const double lm_scale = std::sqrt(std::max(1e-12, 2.0 * xm / T));
            sigma = std::clamp(0.5 * (guess + lm_scale), MIN_SIGMA, 1.0);
        }

        double lo = MIN_SIGMA;
        double flo = f_price(lo);
        if (flo > 0.0) {
            lo = std::max(1e-12, 0.25 * lo);
            flo = f_price(lo);
        }

        double hi = std::clamp(2.0 * sigma, 0.5, 1.0);   // start modestly above seed
        double fhi = f_price(hi);
        int expand_tries = 0;
        while (fhi < 0.0 && hi < MAX_SIGMA && expand_tries < 12) {
            hi *= 1.5;
            fhi = f_price(hi);
            ++expand_tries;
        }

        //last resort :/
        if (!(flo <= 0.0 && fhi >= 0.0)) {
            lo = MIN_SIGMA;
            flo = f_price(lo);
            hi  = MAX_SIGMA;
            fhi = f_price(hi);
            if (!(flo <= 0.0 && fhi >= 0.0)) {
                return {std::numeric_limits<double>::quiet_NaN(), 0, 0, false};
            }
        }

        int newton_iters = 0;
        bool newton_ok = false;
        sigma = std::clamp(sigma, lo, hi);

        for (; newton_iters < MAX_NEWTON_ITERS; ++newton_iters) {
            double f, vega;
            f_price_vega(sigma, f, vega);

            if (std::abs(f) <= price_tol) {
                newton_ok = true;
                break;
            }

            if (f > 0.0) {
                hi = sigma;
            } else {
                lo = sigma;
            }

            if (!std::isfinite(vega) || std::abs(vega) < 1e-12) {
                sigma = 0.5 * (lo + hi);
                continue;
            }

            double step = f / vega;
            double cand = sigma - step;

            if (!(cand > lo && cand < hi) || !std::isfinite(cand)) {
                cand = 0.5 * (lo + hi);
            }

            if (std::abs(cand - sigma) <= vol_tol) {
                sigma = cand;
                newton_ok = true;
                break;
            }
            sigma = cand;
        }

        if (newton_ok) {
            return {std::clamp(sigma, MIN_SIGMA, MAX_SIGMA), newton_iters, 0, true};
        }
        auto f_only = [&](double x) { return f_price(x); };
        auto br = brent(f_only, lo, hi, vol_tol, MAX_BRENT_ITERS);

        return {std::clamp(br.x, MIN_SIGMA, MAX_SIGMA), newton_iters, br.iters, br.converged};
    }

} // namespace vol::bs
