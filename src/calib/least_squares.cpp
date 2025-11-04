#include "libvol/calib/least_squares.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace vol::calib {

// super simple projected gradient "LBFGS-B-like" placeholder so that compiling works
LSQResult lbfgsb(
    const std::vector<double>& x0,
    const std::vector<double>& lb,
    const std::vector<double>& ub,
    const std::function<void(const std::vector<double>&, double&, std::vector<double>&)>& f_grad,
    int maxit,
    double tol)
{
    const int n = static_cast<int>(x0.size());
    std::vector<double> x = x0;
    std::vector<double> g(n, 0.0);

    auto project = [&](std::vector<double>& v){
        for (int i = 0; i < n; ++i) {
            double lo = (i < (int)lb.size() ? lb[i] : -std::numeric_limits<double>::infinity());
            double hi = (i < (int)ub.size() ? ub[i] :  std::numeric_limits<double>::infinity());
            if (v[i] < lo) v[i] = lo;
            if (v[i] > hi) v[i] = hi;
        }
    };

    double f = 0.0;
    f_grad(x, f, g);
    project(x);

    double best_f = f;
    std::vector<double> best_x = x;

    double alpha = 1e-1;   // step
    for (int it = 0; it < maxit; ++it) {
        // gradient norm
        double gn = 0.0;
        for (double v : g) gn += v * v;
        gn = std::sqrt(gn);
        if (gn < tol) {
            return { x, f, it+1, true };
        }

        // take step
        std::vector<double> x_new(n);
        for (int i = 0; i < n; ++i) {
            x_new[i] = x[i] - alpha * g[i];
        }
        project(x_new);

        double f_new = 0.0;
        std::vector<double> g_new;
        f_grad(x_new, f_new, g_new);

        // basic Armijo-ish check
        if (f_new < f) {
            x = std::move(x_new);
            g = std::move(g_new);
            f = f_new;
            if (f < best_f) {
                best_f = f;
                best_x = x;
            }
            // maybe increase step a bit
            alpha = std::min(1.0, alpha * 1.2);
        } else {
            // step too big, shrink
            alpha *= 0.5;
            if (alpha < 1e-10) {
                break;
            }
        }
    }

    return { best_x, best_f, maxit, false };
}

} // namespace vol::calib
