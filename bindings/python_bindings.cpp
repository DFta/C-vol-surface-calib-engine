#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "libvol/models/black_scholes.hpp"
#include "libvol/mc/gbm.hpp"
#include "libvol/models/binom.hpp"



namespace py = pybind11;


PYBIND11_MODULE(volpy, m){
m.doc() = "Volatility & Derivatives pricing (MVP)";

// --- Black-Scholes types ---
py::class_<vol::bs::PriceGreeks>(m, "PriceGreeks")
.def_readonly("price", &vol::bs::PriceGreeks::price)
.def_readonly("delta", &vol::bs::PriceGreeks::delta)
.def_readonly("gamma", &vol::bs::PriceGreeks::gamma)
.def_readonly("vega", &vol::bs::PriceGreeks::vega)
.def_readonly("theta", &vol::bs::PriceGreeks::theta)
.def_readonly("rho", &vol::bs::PriceGreeks::rho);


py::class_<vol::bs::IVResult>(m, "IVResult")
.def_readonly("iv", &vol::bs::IVResult::iv)
.def_readonly("newton_iters", &vol::bs::IVResult::newton_iters)
.def_readonly("brent_iters", &vol::bs::IVResult::brent_iters)
.def_readonly("converged", &vol::bs::IVResult::converged);

// --- Binomial types ---
py::class_<vol::binom::PriceGreeks>(m, "BinomPriceGreeks")
    .def_readonly("price", &vol::binom::PriceGreeks::price)
    .def_readonly("delta", &vol::binom::PriceGreeks::delta)
    .def_readonly("gamma", &vol::binom::PriceGreeks::gamma)
    .def_readonly("vega",  &vol::binom::PriceGreeks::vega)
    .def_readonly("theta", &vol::binom::PriceGreeks::theta)
    .def_readonly("rho",   &vol::binom::PriceGreeks::rho);

py::class_<vol::binom::BinomialResult>(m, "BinomialResult")
    .def_readonly("price", &vol::binom::BinomialResult::price)
    .def_readonly("early_exercise_step", &vol::binom::BinomialResult::early_exercise_step);

// --- Black-Scholes functions ---
m.def("bs_price", &vol::bs::price, "Black-Scholes price",
py::arg("S"),py::arg("K"),py::arg("r"),py::arg("q"),py::arg("T"),py::arg("vol"),py::arg("is_call"));
m.def("bs_price_greeks", &vol::bs::price_greeks, "BS price + Greeks");
m.def("implied_vol", &vol::bs::implied_vol, "Robust implied vol");

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


py::class_<vol::mc::MCResult>(m, "MCResult")
.def_readonly("price", &vol::mc::MCResult::price)
.def_readonly("stderr", &vol::mc::MCResult::std_err)
.def_readonly("paths", &vol::mc::MCResult::paths);


m.def("mc_euro_gbm", &vol::mc::european_vanilla_gbm, "Monte Carlo GBM pricer");
}


