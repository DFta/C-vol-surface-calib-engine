#include "libvol/models/black_scholes.hpp"
#include "libvol/math/root_finders.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace vol::bs {
    // ------------------------------
    // Implied volatility via Newton (safeguarded) + Brent fallback
    // ------------------------------
    [[nodiscard]] inline IVResult implied_vol(double S, double K, double r, double q,double T, double target, bool is_call,double init, double tol){
        using namespace vol::root;

        // ---- Basic input validation
        if (!(std::isfinite(S) && std::isfinite(K) && std::isfinite(r) && std::isfinite(q) && std::isfinite(T) && std::isfinite(target))) {
            return {std::numeric_limits<double>::quiet_NaN(), 0, 0, false};
        }
        if (S <= 0.0 || K <= 0.0 || T <= 0.0 || target < 0.0) {
            return {std::numeric_limits<double>::quiet_NaN(), 0, 0, false};
        }

        // ---- Useful constants
        constexpr double PI = 3.141592653589793238462643383279502884;
        constexpr double MIN_SIGMA = 1e-9;     // strictly positive lower bound
        constexpr double MAX_SIGMA = 5.0;      // very large but finite
        constexpr int    MAX_NEWTON_ITERS = 20;
        constexpr int    MAX_BRENT_ITERS  = 100;

        const double df_r = std::exp(-r * T);
        const double df_q = std::exp(-q * T);
        const double F    = S * std::exp((r - q) * T); // forward

        // Intrinsic and hard bounds (BS price is increasing in sigma)
        const double intrinsic = (is_call
            ? std::max(0.0, S * df_q - K * df_r)
            : std::max(0.0, K * df_r - S * df_q));

        const double max_price = is_call ? S * df_q : K * df_r;

        // Tight relative eps for boundary checks
        const double rel_eps = 1e-10;

        if (target < intrinsic * (1.0 - rel_eps)) {
            // Below intrinsic is impossible for a no-arb European option
            return {std::numeric_limits<double>::quiet_NaN(), 0, 0, false};
        }
        if (target <= intrinsic * (1.0 + 1e-4)) {
            // At (or extremely close to) intrinsic -> sigma ~ 0
            return {0.0, 0, 0, true};
        }
        if (target > max_price * (1.0 + rel_eps)) {
            // Above the discounted bound is impossible
            return {std::numeric_limits<double>::quiet_NaN(), 0, 0, false};
        }
        if (target >= max_price * (1.0 - 1e-12)) {
            // Practically at the bound; the IV would be enormous/undefined
            return {std::numeric_limits<double>::quiet_NaN(), 0, 0, false};
        }

        // Tighten tolerance for very short maturities to avoid chasing the noise
        if (T < 1.0 / 365.0) {
            tol = std::max(tol, 1e-6);
        }
        const double vol_tol = std::max(1e-12, tol);
        const double price_tol = std::max(1e-12, 1e-8 * std::max(1.0, target));

        // ---- Helper lambdas
        auto f_price = [&](double sigma) -> double {
            return price(S, K, r, q, T, sigma, is_call) - target;
        };

        auto f_price_vega = [&](double sigma, double& f, double& vega) {
            auto g = price_greeks(S, K, r, q, T, sigma, is_call);
            f    = g.price - target;
            vega = g.vega;           // per 1.0 vol (not %)
        };

        // ---- Initial guess
        double sigma = std::numeric_limits<double>::quiet_NaN();
        const bool init_ok = (init > 0.0 && init < MAX_SIGMA && std::isfinite(init));

        if (init_ok) {
            sigma = init;
        } else {
            // Brenner–Subrahmanyam ATM-ish guess; cheap and decent
            // Normalize to underlying-side for call / strike-side for put isn't necessary;
            // using S-side works well as a general quick seed.
            const double c_norm = target / (S * df_q);
            const double guess  = std::sqrt(std::max(1e-12, (2.0 * PI / T))) * std::max(1e-12, c_norm);
            // Also blend with log-moneyness-based scale to help off-ATM
            const double xm = std::abs(std::log((F + 1e-300) / (K + 1e-300)));
            const double lm_scale = std::sqrt(std::max(1e-12, 2.0 * xm / T));
            sigma = std::clamp(0.5 * (guess + lm_scale), MIN_SIGMA, 1.0);
        }

        // ---- Build a bracket [lo, hi] with f(lo) <= 0 <= f(hi)
        // At sigma -> 0, price -> intrinsic, so f(lo) <= 0 (we already checked target >= intrinsic)
        double lo = MIN_SIGMA;
        double flo = f_price(lo);
        if (flo > 0.0) {
            // Numerical oddity: push lo down a bit more
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

        if (!(flo <= 0.0 && fhi >= 0.0)) {
            // As a last resort, lay down a very wide bracket
            lo = MIN_SIGMA;
            flo = f_price(lo);
            hi  = MAX_SIGMA;
            fhi = f_price(hi);
            if (!(flo <= 0.0 && fhi >= 0.0)) {
                // No sign change: cannot safely bracket (should be extremely rare)
                return {std::numeric_limits<double>::quiet_NaN(), 0, 0, false};
            }
        }

        // ---- Safeguarded Newton within the bracket
        int newton_iters = 0;
        bool newton_ok   = false;
        sigma            = std::clamp(sigma, lo, hi);

        for (; newton_iters < MAX_NEWTON_ITERS; ++newton_iters) {
            double f, vega;
            f_price_vega(sigma, f, vega);

            if (std::abs(f) <= price_tol) {
                newton_ok = true;
                break;
            }

            // Maintain the bracket based on sign
            if (f > 0.0) {
                hi = sigma;
            } else {
                lo = sigma;
            }

            if (!std::isfinite(vega) || std::abs(vega) < 1e-12) {
                // Vega too small: switch to bisection step
                sigma = 0.5 * (lo + hi);
                continue;
            }

            // Newton step
            double step = f / vega;
            double cand = sigma - step;

            // Safeguard: if Newton attempts to leave the bracket, bisect instead
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

        // ---- Brent fallback on the established bracket
        auto f_only = [&](double x) { return f_price(x); };
        auto br = brent(f_only, lo, hi, vol_tol, MAX_BRENT_ITERS);

        return {std::clamp(br.x, MIN_SIGMA, MAX_SIGMA), newton_iters, br.iters, br.converged};
    }

} // namespace vol::bs
