#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "libvol/models/black_scholes.hpp"
#include "libvol/mc/gbm.hpp"
#include "libvol/models/binom.hpp"
#include "libvol/models/heston.hpp"
#include "libvol/models/svi.hpp"
#include "libvol/calib/svi_slice.hpp"
#include "libvol/core/types.hpp"

namespace py = pybind11;

PYBIND11_MODULE(volpy, m) {
    m.doc() = "Volatility & Derivatives pricing (MVP)";

    // --- Black-Scholes types ---
    py::class_<vol::bs::PriceGreeks>(m, "PriceGreeks")
        .def_readonly("price", &vol::bs::PriceGreeks::price)
        .def_readonly("delta", &vol::bs::PriceGreeks::delta)
        .def_readonly("gamma", &vol::bs::PriceGreeks::gamma)
        .def_readonly("vega",  &vol::bs::PriceGreeks::vega)
        .def_readonly("theta", &vol::bs::PriceGreeks::theta)
        .def_readonly("rho",   &vol::bs::PriceGreeks::rho);

    py::class_<vol::bs::IVResult>(m, "IVResult")
        .def_readonly("iv",           &vol::bs::IVResult::iv)
        .def_readonly("newton_iters", &vol::bs::IVResult::newton_iters)
        .def_readonly("brent_iters",  &vol::bs::IVResult::brent_iters)
        .def_readonly("converged",    &vol::bs::IVResult::converged);

    // --- Binomial types ---
    py::class_<vol::binom::PriceGreeks>(m, "BinomPriceGreeks")
        .def_readonly("price", &vol::binom::PriceGreeks::price)
        .def_readonly("delta", &vol::binom::PriceGreeks::delta)
        .def_readonly("gamma", &vol::binom::PriceGreeks::gamma)
        .def_readonly("vega",  &vol::binom::PriceGreeks::vega)
        .def_readonly("theta", &vol::binom::PriceGreeks::theta)
        .def_readonly("rho",   &vol::binom::PriceGreeks::rho);

    py::class_<vol::binom::BinomialResult>(m, "BinomialResult")
        .def_readonly("price",              &vol::binom::BinomialResult::price)
        .def_readonly("early_exercise_step",&vol::binom::BinomialResult::early_exercise_step);

    // --- Black-Scholes functions ---
    m.def("bs_price",
        &vol::bs::price,
        "Black-Scholes price",
        py::arg("S"), py::arg("K"), py::arg("r"), py::arg("q"),
        py::arg("T"), py::arg("vol"), py::arg("is_call"));

    m.def("bs_price_greeks",
        &vol::bs::price_greeks,
        "BS price + Greeks");

    m.def("implied_vol",
    &vol::bs::implied_vol,
    "Robust implied vol",
    py::arg("S"),
    py::arg("K"),
    py::arg("r"),
    py::arg("q"),
    py::arg("T"),
    py::arg("target"),
    py::arg("is_call"),
    py::arg("init") = 0.2,
    py::arg("tol")  = 1e-10);


    // --- Binomial functions ---
    m.def("binom_price",
        &vol::binom::price,
        "CRR binomial price",
        py::arg("S"), py::arg("K"), py::arg("r"), py::arg("q"),
        py::arg("T"), py::arg("vol"), py::arg("steps"),
        py::arg("is_call"), py::arg("is_american"));

    m.def("binom_price_w_info",
        &vol::binom::price_w_info,
        "CRR binomial price + earliest early-exercise step",
        py::arg("S"), py::arg("K"), py::arg("r"), py::arg("q"),
        py::arg("T"), py::arg("vol"), py::arg("steps"),
        py::arg("is_call"), py::arg("is_american"));

    m.def("binom_price_greeks",
        &vol::binom::price_greeks,
        "CRR binomial price + Greeks (finite differences)",
        py::arg("S"), py::arg("K"), py::arg("r"), py::arg("q"),
        py::arg("T"), py::arg("vol"), py::arg("steps"),
        py::arg("is_call"), py::arg("is_american"));

    // --- Monte Carlo ---
    py::class_<vol::mc::MCResult>(m, "MCResult")
        .def_readonly("price",  &vol::mc::MCResult::price)
        .def_readonly("stderr", &vol::mc::MCResult::std_err)
        .def_readonly("paths",  &vol::mc::MCResult::paths);

    m.def("mc_euro_gbm",
        &vol::mc::european_vanilla_gbm,
        "Monte Carlo GBM pricer");

    // --- Heston model ---
    py::class_<vol::heston::Params>(m, "HestonParams")
        .def(py::init<double,double,double,double,double>(),
            py::arg("kappa"), py::arg("theta"), py::arg("sigma"),
            py::arg("rho"), py::arg("v0"))
        .def_readwrite("kappa", &vol::heston::Params::kappa)
        .def_readwrite("theta", &vol::heston::Params::theta)
        .def_readwrite("sigma", &vol::heston::Params::sigma)
        .def_readwrite("rho", &vol::heston::Params::rho)
        .def_readwrite("v0", &vol::heston::Params::v0);

    m.def("heston_price_cf",
        &vol::heston::price_cf,
        "Carr-Madan/Attari vanilla price via Gauss-Laguerre integration",
        py::arg("S"), py::arg("K"), py::arg("r"), py::arg("q"),
        py::arg("T"), py::arg("params"), py::arg("is_call"),
        py::arg("n_gl") = 64);

    // --- SVI params ---
    py::class_<vol::svi::Params>(m, "SVIParams")
        .def(py::init<>())
        .def("__repr__", [](const vol::svi::Params& p) {
            return "SVIParams{a=" + std::to_string(p[0]) +
                ", b=" + std::to_string(p[1]) +
                ", rho=" + std::to_string(p[2]) +
                ", m=" + std::to_string(p[3]) +
                ", sigma=" + std::to_string(p[4]) + "}";
        });

    m.def("svi_total_variance",
        &vol::svi::total_variance,
        "SVI total variance w(k) for raw params");

    m.def("svi_basic_no_arb",
        &vol::svi::basic_no_arb,
        "Basic SVI sanity checks");

    // --- Market data types for SVI ---
    py::class_<vol::OptionSpec>(m, "OptionSpec")
        .def(py::init<double,double,double,double,double,bool>(),
            py::arg("S"), py::arg("K"), py::arg("r"),
            py::arg("q"), py::arg("T"), py::arg("is_call"))
        .def_readwrite("S", &vol::OptionSpec::S)
        .def_readwrite("K", &vol::OptionSpec::K)
        .def_readwrite("r", &vol::OptionSpec::r)
        .def_readwrite("q", &vol::OptionSpec::q)
        .def_readwrite("T", &vol::OptionSpec::T)
        .def_readwrite("is_call", &vol::OptionSpec::is_call);

    py::class_<vol::svi::SliceConfig>(m, "SliceConfig")
        .def(py::init<>())
        .def_readwrite("use_vega_weights", &vol::svi::SliceConfig::use_vega_weights)
        .def_readwrite("wing_dampen_pow", &vol::svi::SliceConfig::wing_dampen_pow)
        .def_readwrite("min_vega_eps", &vol::svi::SliceConfig::min_vega_eps)
        .def_readwrite("min_points", &vol::svi::SliceConfig::min_points);

    // Calibrate a slice directly from (OptionSpec[], mids[])
    m.def("svi_calibrate_slice_from_prices",
        &vol::svi::calibrate_slice_from_prices,
        py::arg("opts"),
        py::arg("mids"),
        py::arg("cfg") = vol::svi::SliceConfig{});
}
