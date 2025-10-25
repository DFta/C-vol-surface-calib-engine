#include "libvol/models/binom.hpp"
#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>

namespace vol::binom {

namespace {
    inline double intrinsic(bool is_call, double S, double K) {
        return is_call ? std::max(0.0, S - K) : std::max(0.0, K - S);
    }

    struct CRRParams {
        double dt;
        double u, d, p, disc;
    };

    inline CRRParams make_crr(double r, double q, double T, double vol, int steps) {
        const double dt = T / static_cast<double>(steps);
        const double vs = vol * std::sqrt(dt);
        const double u  = std::exp(vs);
        const double d  = 1.0 / u;
        const double a  = std::exp((r - q) * dt);
        const double p  = (a - d) / (u - d);
        const double disc = std::exp(-r * dt);
        return {dt, u, d, p, disc};
    }

    /* 
    Core CRR engine. If early_exercise_step_out != nullptr, it will be filled
    with the earliest step (0..steps-1) where early exercise is optimal at any node; 
    or -1 if never optimal. Note this is the earliest layer where
    early exercise occurs anywhere, not necessarily along a single optimal path.
    */
    double price_crr(double S0, double K, double r, double q, double T, double vol,
                    int steps, bool is_call, bool is_american, int* early_ex_step_out)
    {
        if (steps <= 0) {
            // degenerate: fallback to intrinsic at expiry
            return std::exp(-r*T) * intrinsic(is_call, S0 * std::exp((r - q - 0.5*vol*vol)*T), K);
        }
        const auto p = make_crr(r, q, T, vol, steps);
        double prob = p.p;
        if (!(prob >= 0.0 && prob <= 1.0) || !std::isfinite(prob)) {
            prob = std::min(1.0, std::max(0.0, prob));
        }       

        // Precompute terminal asset prices and option values
        std::vector<double> V(steps + 1);
        std::vector<double> S(steps + 1);

        // S_N(j) = S0 * u^j * d^(N-j)
        double Sj = S0 * std::pow(p.d, steps);
        const double ud = p.u / p.d; // = u^2 since d=1/u
        for (int j = 0; j <= steps; ++j) {
            S[j] = Sj;
            V[j] = intrinsic(is_call, S[j], K);
            Sj *= ud;
        }

        int earliest_ex_step = -1;

        // Backward induction
        for (int n = steps - 1; n >= 0; --n) {
            for (int j = 0; j <= n; ++j) {
                const double cont = p.disc * (prob * V[j + 1] + (1.0 - prob) * V[j]);
                if (is_american) {
                    // Recompute S at this layer/node from child
                    const double S_nj = S[j] / p.d; // going one step back is divide by d
                    const double exer = intrinsic(is_call, S_nj, K);
                    const double vn = std::max(cont, exer);
                    if (earliest_ex_step == -1 && exer > cont + 1e-16) {
                        // note: numerical tolerance
                        earliest_ex_step = n;
                    }
                    V[j] = vn;
                    S[j] = S_nj; // maintain S for next loop
                } else {
                    V[j] = cont;
                    S[j] = S[j] / p.d;
                }
            }
        }

        if (early_ex_step_out) *early_ex_step_out = earliest_ex_step;
        return V[0];
    }

    inline double safe_dt_for_theta(double T) {
        // keep finite difference stable vs. very short maturities
        const double min_dt  = 1.0 / 3650.0;       // ~0.1 day
        const double frac_T  = 0.05 * T;           // 5% of T
        const double max_abs = 5.0 / 365.0;        // cap at ~5 days
        return std::min(std::max(min_dt, frac_T), std::max(min_dt, max_abs));
    }
} // namespace

double price(double S, double K, double r, double q, double T, double vol, int steps, bool is_call, bool is_american)
{
    return price_crr(S, K, r, q, T, vol, steps, is_call, is_american, nullptr);
}

BinomialResult price_w_info(double S, double K, double r, double q, double T,
                            double vol, int steps, bool is_call, bool is_american)
{
    int earliest = -1;
    const double px = price_crr(S, K, r, q, T, vol, steps, is_call, is_american, &earliest);
    return {px, earliest};
}

PriceGreeks price_greeks(double S, double K, double r, double q, double T, double vol, int steps, bool is_call, bool is_american)
{
    // Base price
    const double base = price(S, K, r, q, T, vol, steps, is_call, is_american);

    // Bump sizes (relative when sensible)
    const double hS   = std::max(1e-8, 1e-4 * std::max(1.0, S));
    const double hvol = std::max(1e-8, 1e-4 * std::max(1.0, vol));
    const double hr   = std::max(1e-8, 1e-5 * std::max(1.0, std::fabs(r)));
    double hT         = safe_dt_for_theta(T); // years

    // Delta & Gamma (central on S)
    const double p_sp = price(S + hS, K, r, q, T, vol, steps, is_call, is_american);
    const double p_sm = price(std::max(1e-12, S - hS), K, r, q, T, vol, steps, is_call, is_american);
    const double delta = (p_sp - p_sm) / (2.0 * hS);
    const double gamma = (p_sp - 2.0 * base + p_sm) / (hS * hS);

    // Vega (central on vol)
    const double p_vp = price(S, K, r, q, T, vol + hvol, steps, is_call, is_american);
    const double p_vm = price(S, K, r, q, T, std::max(1e-12, vol - hvol), steps, is_call, is_american);
    const double vega = (p_vp - p_vm) / (2.0 * hvol);

    // Rho (central on r)
    const double p_rp = price(S, K, r + hr, q, T, vol, steps, is_call, is_american);
    const double p_rm = price(S, K, std::max(-0.999, r - hr), q, T, vol, steps, is_call, is_american);
    const double rho  = (p_rp - p_rm) / (2.0 * hr);

    // Theta (central on T; note theta is ∂V/∂T, i.e., change per year)
    // If T is very small, fall back to one-sided forward difference.
    double theta;
    if (T > hT) {
        const double p_tp = price(S, K, r, q, T + hT, vol, steps, is_call, is_american);
        const double p_tm = price(S, K, r, q, T - hT, vol, steps, is_call, is_american);
        theta = (p_tp - p_tm) / (2.0 * hT);
    } else {
        const double p_tp = price(S, K, r, q, T + hT, vol, steps, is_call, is_american);
        theta = (p_tp - base) / hT;
    }

    return {base, delta, gamma, vega, theta, rho};
}

} // namespace vol::binom