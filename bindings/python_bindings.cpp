#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "libvol/models/black_scholes.hpp"
#include "libvol/mc/gbm.hpp"


namespace py = pybind11;


PYBIND11_MODULE(volpy, m){
m.doc() = "Volatility & Derivatives pricing (MVP)";


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


m.def("bs_price", &vol::bs::price, "Black-Scholes price",
py::arg("S"),py::arg("K"),py::arg("r"),py::arg("q"),py::arg("T"),py::arg("vol"),py::arg("is_call"));
m.def("bs_price_greeks", &vol::bs::price_greeks, "BS price + Greeks");
m.def("implied_vol", &vol::bs::implied_vol, "Robust implied vol");


py::class_<vol::mc::MCResult>(m, "MCResult")
.def_readonly("price", &vol::mc::MCResult::price)
.def_readonly("stderr", &vol::mc::MCResult::std_err)
.def_readonly("paths", &vol::mc::MCResult::paths);


m.def("mc_euro_gbm", &vol::mc::european_vanilla_gbm, "Monte Carlo GBM pricer");
}