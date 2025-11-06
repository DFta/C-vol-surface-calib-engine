#include "libvol/models/svi.hpp"
#include "libvol/calib/least_squares.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>

namespace vol::svi {

    // p = {a, b, rho, m, sigma}
    double total_variance(double k, const Params& p) {
        const double a = p[0], b = p[1], rho = p[2], m = p[3], sigma = p[4];
        const double x = k - m;
        const double R = std::sqrt(x * x + sigma * sigma);
        return a + b * (rho * x + R);
    }

    bool basic_no_arb(const Params& p) {
        const double b = p[1], rho = p[2], sigma = p[4];
        if (!(std::isfinite(b) && std::isfinite(rho) && std::isfinite(sigma))) return false;
        if (b <= 0.0) return false;
        if (std::abs(rho) >= 1.0) return false;
        if (sigma <= 0.0) return false;
        return true;
    }

    static inline double clamp(double x, double lo, double hi) {
        return std::max(lo, std::min(hi, x));
    }

    static inline void linreg_slope(const std::vector<double>& x, const std::vector<double>& y, std::size_t i0, std::size_t i1, double& slope_out) {
        // fit y = s*x + c on [i0, i1] (inclusive)
        const std::size_t n = (i1 >= i0) ? (i1 - i0 + 1) : 0;
        if (n < 2) { slope_out = 0.0; return; }
        double sx = 0.0, sy = 0.0, sxx = 0.0, sxy = 0.0;
        for (std::size_t i = i0; i <= i1; ++i) {
            sx  += x[i];
            sy  += y[i];
            sxx += x[i] * x[i];
            sxy += x[i] * y[i];
        }
        const double n_d = static_cast<double>(n);
        const double den = (n_d * sxx - sx * sx);
        slope_out = (std::abs(den) > 0.0) ? (n_d * sxy - sx * sy) / den : 0.0;
    }

    static inline double local_quadratic_curvature(const std::vector<double>& k, const std::vector<double>& w, std::size_t idx_min) {
        const std::size_t n = k.size();
        if (n < 3) return 0.0;
        std::vector<std::pair<double,double>> pts;
        pts.reserve(std::min<std::size_t>(5, n));

        std::vector<std::pair<double,std::size_t>> dist_idx;
        dist_idx.reserve(n);
        const double km = k[idx_min];
        for (std::size_t i = 0; i < n; ++i)
            dist_idx.emplace_back(std::abs(k[i] - km), i);
        std::sort(dist_idx.begin(), dist_idx.end(), [](auto& a, auto& b){ return a.first < b.first; });

        const std::size_t take = std::min<std::size_t>(5, n);
        for (std::size_t t = 0; t < take; ++t) {
            auto j = dist_idx[t].second;
            pts.emplace_back(k[j] - km, w[j]); // x = k - km
        }
        if (pts.size() < 3) return 0.0;

        double S0=0, S1=0, S2=0, S3=0, S4=0, Sy=0, Sxy=0, Sx2y=0;
        for (auto& pr : pts) {
            const double x = pr.first, y = pr.second;
            const double x2 = x*x, x3 = x2*x, x4 = x2*x2;
            S0 += 1.0; S1 += x; S2 += x2; S3 += x3; S4 += x4;
            Sy += y;   Sxy += x*y; Sx2y += x2*y;
        }

        double M[3][4] = {
            {S0, S1, S2, Sy   },
            {S1, S2, S3, Sxy  },
            {S2, S3, S4, Sx2y }
        };
        // Gaussian elimination
        for (int col = 0; col < 3; ++col) {
            // pivot
            int piv = col;
            for (int r = col + 1; r < 3; ++r)
                if (std::abs(M[r][col]) > std::abs(M[piv][col])) piv = r;
            if (std::abs(M[piv][col]) < 1e-14) return 0.0; 
            if (piv != col) for (int c = col; c < 4; ++c) std::swap(M[piv][c], M[col][c]);
            const double inv = 1.0 / M[col][col];
            for (int c = col; c < 4; ++c) M[col][c] *= inv;
            for (int r = 0; r < 3; ++r) if (r != col) {
                const double f = M[r][col];
                for (int c = col; c < 4; ++c) M[r][c] -= f * M[col][c];
            }
        }
        const double A = M[0][3];
        const double B = M[1][3];
        const double halfC = M[2][3];
        (void)A; (void)B;
        return 2.0 * halfC; // C
    }

// Per-slice
    Params fit_raw_svi(const std::vector<double>& k, const std::vector<double>& w_mkt, const std::vector<double>& wts_in)
    {
        const std::size_t n = std::min(k.size(), w_mkt.size());
        std::vector<double> kx(k.begin(), k.begin() + n);
        std::vector<double> wy(w_mkt.begin(), w_mkt.begin() + n);
        if (n < 5) {
            const double kmin = *std::min_element(kx.begin(), kx.end());
            const double kmax = *std::max_element(kx.begin(), kx.end());
            const double wmin = *std::min_element(wy.begin(), wy.end());
            const double b0 = 0.1;
            const double sigma0 = std::max(1e-3, 0.2 * (kmax - kmin));
            const double a0 = std::max(1e-10, wmin - b0 * sigma0);
            return Params{ a0, b0, 0.0, 0.5 * (kmin + kmax), sigma0 };
        }

        std::vector<std::size_t> idx(n);
        std::iota(idx.begin(), idx.end(), 0);
        std::sort(idx.begin(), idx.end(), [&](std::size_t i, std::size_t j){ return kx[i] < kx[j]; });

        std::vector<double> k_sorted(n), w_sorted(n), wt(n, 1.0);
        for (std::size_t t = 0; t < n; ++t) {
            k_sorted[t] = kx[idx[t]];
            w_sorted[t] = wy[idx[t]];
            if (!wts_in.empty()) wt[t] = wts_in[idx[t]];
        }

        const double kmin = k_sorted.front();
        const double kmax = k_sorted.back();
        const double wrange = (*std::max_element(w_sorted.begin(), w_sorted.end()) -
                            *std::min_element(w_sorted.begin(), w_sorted.end()));
        const double range_k = std::max(1e-6, kmax - kmin);

        std::size_t i_min = 0;
        for (std::size_t i = 1; i < n; ++i) if (w_sorted[i] < w_sorted[i_min]) i_min = i;
        const double m0 = k_sorted[i_min];
        const double w_at_min = w_sorted[i_min];

        const std::size_t wing = std::max<std::size_t>(2, n / 5);
        double sL = 0.05, sR = 0.05;
        linreg_slope(k_sorted, w_sorted, 0, std::min(wing, n-1), sL);
        linreg_slope(k_sorted, w_sorted, (n > wing ? n - wing : 0), n - 1, sR);
        sL = std::max(1e-4, std::abs(sL));
        sR = std::max(1e-4, std::abs(sR));

        double b0   = 0.5 * (sL + sR);
        double rho0 = (sR - sL) / std::max(1e-12, (sR + sL));
        b0   = clamp(b0,   1e-6, 10.0);
        rho0 = clamp(rho0, -0.95, 0.95);

        double c2 = local_quadratic_curvature(k_sorted, w_sorted, i_min);
        double sigma0 = (c2 > 1e-6) ? clamp(b0 / c2, 1e-4, 2.0) : clamp(0.2 * range_k, 1e-4, 2.0);

        double a0 = std::max(1e-10, w_at_min - b0 * sigma0);

        const double a_max = std::max(1.0, 5.0 * (wrange > 0.0 ? wrange : (w_at_min + b0 * sigma0 + 1.0)));
        const std::vector<double> x0 = { a0, b0, rho0, m0, sigma0 };
        const std::vector<double> lb = { 1e-12, 1e-8, -0.999, kmin - 1.0 * range_k, 1e-6 };
        const std::vector<double> ub = { a_max,  10.0,  0.999, kmax + 1.0 * range_k,  5.0  };

        auto f_grad = [&](const std::vector<double>& x, double& f, std::vector<double>& g) {
            const double a = x[0], b = x[1], rho = x[2], m = x[3], sigma = x[4];
            const double eps = 1e-12;
            const double rho_c = clamp(rho, -0.999, 0.999);
            const double b_pos = std::max(b, 1e-12);
            const double s_pos = std::max(sigma, 1e-12);

            double sumw = 0.0, obj = 0.0;
            double ga = 0.0, gb = 0.0, gr = 0.0, gm = 0.0, gs = 0.0;

            for (std::size_t i = 0; i < n; ++i) {
                const double wi = (i < wt.size() ? std::max(0.0, wt[i]) : 1.0);
                if (wi <= 0.0) continue;

                const double xk = k_sorted[i] - m;
                const double R  = std::sqrt(xk * xk + s_pos * s_pos);
                const double w_model = a + b_pos * (rho_c * xk + R);
                const double r  = (w_model - w_sorted[i]); // residual in total variance

                sumw += wi;
                obj  += wi * r * r;

                const double dw_da = 1.0;
                const double dw_db = (rho_c * xk + R);
                const double dw_dr = b_pos * xk;
                const double dw_dm = b_pos * (-rho_c - xk / std::max(R, eps));
                const double dw_ds = b_pos * (s_pos / std::max(R, eps));

                ga += wi * r * dw_da;
                gb += wi * r * dw_db;
                gr += wi * r * dw_dr;
                gm += wi * r * dw_dm;
                gs += wi * r * dw_ds;
            }

            if (sumw <= 0.0) { f = 0.0; g.assign(5, 0.0); return; }

            const double inv = 1.0 / sumw;
            f = 0.5 * obj * inv;

            g.resize(5);
            g[0] = ga * inv;
            g[1] = gb * inv;
            g[2] = gr * inv;
            g[3] = gm * inv;
            g[4] = gs * inv;

            double pen = 0.0;
            if (b <= 0.0)      { pen += (1.0 - std::tanh( 100.0 * b));       g[1] += -100.0 * inv; }
            if (std::abs(rho) >= 1.0){ pen += std::tanh(100.0 * (std::abs(rho) - 0.999)); g[2] +=  100.0 * inv * ((rho > 0) ? 1.0 : -1.0); }
            if (sigma <= 0.0)  { pen += (1.0 - std::tanh( 100.0 * sigma));   g[4] += -100.0 * inv; }
            f += 1e-8 * pen;
        };

        Params best_p{};
        double best_rmse = std::numeric_limits<double>::infinity();
        const double base_tol = 1e-8;

        const std::vector<std::vector<double>> starts = {
            x0,
            { x0[0], x0[1], clamp(x0[2] + 0.2, -0.95, 0.95), clamp(x0[3] + 0.25 * range_k, kmin - range_k, kmax + range_k), x0[4] },
            { x0[0], x0[1], clamp(x0[2] - 0.2, -0.95, 0.95), clamp(x0[3] - 0.25 * range_k, kmin - range_k, kmax + range_k), x0[4] }
        };

        for (const auto& s : starts) {
            auto res = calib::lbfgsb(s, lb, ub, f_grad, 500, base_tol);
            if (res.converged && res.x.size() == 5) {
                const double rmse = std::sqrt(std::max(0.0, res.rmse));
                if (rmse < best_rmse) {
                    best_rmse = rmse;
                    best_p = Params{ res.x[0], res.x[1], res.x[2], res.x[3], res.x[4] };
                }
            }
        }

        if (!basic_no_arb(best_p)) {
            best_p = Params{ clamp(x0[0], lb[0], ub[0]),
                            clamp(x0[1], lb[1], ub[1]),
                            clamp(x0[2], lb[2], ub[2]),
                            clamp(x0[3], lb[3], ub[3]),
                            clamp(x0[4], lb[4], ub[4]) };
        }

        return best_p;
    }

} // namespace vol::svi
