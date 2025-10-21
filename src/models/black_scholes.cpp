#include "libvol/models/black_scholes.hpp"
#include "libvol/core/constants.hpp"
#include <algorithm>
#include <cmath>

namespace vol::bs {

static inline double expm1_safe(double x){
    return (std::abs(x) < 1e-6) ? x + 0.5*x*x : std::expm1(x);
}

double phi(double x){ return std::exp(-0.5*x*x) * vol::INV_SQRT2PI; }
double Phi(double x){ return 0.5 * std::erfc(-x/std::sqrt(2.0)); }

static inline double d1(double S,double K,double r,double q,double T,double v){
    return (std::log(S/K) + (r - q + 0.5*v*v)*T)/(v*std::sqrt(T));
}
static inline double d2(double d1,double v,double T){ return d1 - v*std::sqrt(T); }

static inline double forward(double S,double r,double q,double T){
    return S*std::exp((r-q)*T);
}

double price(double S,double K,double r,double q,double T,double v,bool is_call){
    if (T <= 0) {
        double intrinsic = is_call ? std::max(0.0, S-K) : std::max(0.0, K-S);
        return intrinsic;
    }
    const double F    = forward(S,r,q,T);
    const double disc = std::exp(-r*T);
    const double d_1  = d1(S,K,r,q,T,v);
    const double d_2  = d2(d_1,v,T);
    if (is_call) return disc*(F*Phi(d_1) - K*Phi(d_2));
    return          disc*(K*Phi(-d_2) - F*Phi(-d_1));
}

PriceGreeks price_greeks(double S,double K,double r,double q,double T,double v,bool is_call){
    if (T <= 0.0 || v <= 0.0) {
        const double p = price(S,K,r,q,T,v,is_call);
        return {
            p,
            (is_call ? (S > K ? 1.0 : 0.0) : (S < K ? -1.0 : 0.0)),
            0.0, 0.0, 0.0, 0.0
        };
    }

    const double disc = std::exp(-r*T);
    const double sqT  = std::sqrt(T);
    const double d_1  = d1(S,K,r,q,T,v);
    const double d_2  = d2(d_1,v,T);
    const double nd1  = phi(d_1);

    const double call_delta = std::exp(-q*T)*Phi(d_1);
    const double delta = is_call ? call_delta : (call_delta - std::exp(-q*T));

    const double gamma = std::exp(-q*T)*nd1/(S*v*sqT);
    const double vega  = S*std::exp(-q*T)*nd1*sqT;

    const double theta_call = -0.5*S*std::exp(-q*T)*nd1*v/sqT + q*S*std::exp(-q*T)*Phi(d_1) - r*K*disc*Phi(d_2);
    const double theta_put  = -0.5*S*std::exp(-q*T)*nd1*v/sqT - q*S*std::exp(-q*T)*Phi(-d_1) + r*K*disc*Phi(-d_2);
    const double theta = is_call ? theta_call : theta_put;

    const double rho_call = T*K*disc*Phi(d_2);
    const double rho_put  = -T*K*disc*Phi(-d_2);
    const double rho = is_call ? rho_call : rho_put;

    const double p = price(S,K,r,q,T,v,is_call);
    return { p, delta, gamma, vega, theta, rho };
}


} // namespace vol::bs
