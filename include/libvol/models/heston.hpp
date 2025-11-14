#pragma once

#include <cstddef>

namespace vol::heston {

struct Params {
    double kappa;
    double theta;
    double sigma;
    double rho;
    double v0;
};

// Closed-form (Fourier / Carr-Madan) vanilla price via Gauss-Laguerre integration.
// n_gl controls the quadrature order (number of points). Defaults to 64.
double price_cf(double S,
                double K,
                double r,
                double q,
                double T,
                const Params& params,
                bool is_call,
                int n_gl = 64);

} // namespace vol::heston
