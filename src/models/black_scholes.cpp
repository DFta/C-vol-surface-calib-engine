#include "libvol/models/black_scholes.hpp"
#include "libvol/core/constants.hpp"
#include <algorithm>
#include <cmath>

namespace vol::bs {

double phi(double x){ return std::exp(-0.5*x*x) * vol::INV_SQRT2PI; }
double Phi(double x){ return 0.5 * std::erfc(-x/std::sqrt(2.0)); }

static inline double d1(double S,double K,double r,double q,double T,double v, double sqT){
    return (std::log(S/K) + (r - q + 0.5*v*v)*T)/(v*sqT);
}
static inline double d2(double d1,double v,double T,double sqT){ return d1 - v*sqT; }

static inline double forward(double S,double r,double q,double T){
    return S*std::exp((r-q)*T);
}

double price(double S,double K,double r,double q,double T,double v,bool is_call){
    if (T <= 0) {
        double intrinsic = is_call ? std::max(0.0, S-K) : std::max(0.0, K-S);
        return intrinsic;
    }
    const double sqT  = std::sqrt(T);
    const double F    = forward(S,r,q,T);
    const double disc = std::exp(-r*T);
    const double d_1  = d1(S,K,r,q,T,v,sqT);
    const double d_2  = d2(d_1,v,T,sqT);
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

    // defs for efficiency
    const double disc = std::exp(-r*T);
    const double K_disc = K*disc; 
    const double div_disc = std::exp(-q*T); 
    const double sqT  = std::sqrt(T);
    const double d_1  = d1(S,K,r,q,T,v,sqT);
    const double d_2  = d2(d_1,v,T,sqT);
    const double nd1  = phi(d_1);
    const double div_disc_nd1 = div_disc * nd1;
    const double phi_d1 = Phi(d_1);
    const double phi_neg_d1 = -1.0 * phi_d1;
    const double phi_d2 = Phi(d_2);
    const double phi_neg_d2 = -1.0 * phi_d2;

    const double call_delta = div_disc*Phi(d_1);
    const double delta = is_call ? call_delta : (call_delta - div_disc);

    const double gamma = div_disc_nd1/(S*v*sqT);
    const double vega  = S*div_disc_nd1*sqT;

    const double theta_call = -0.5*S*div_disc_nd1*v/sqT + q*S*div_disc*phi_d1 - r*K_disc*phi_d2;
    const double theta_put  = -0.5*S*div_disc_nd1*v/sqT - q*S*div_disc*phi_neg_d1 + r*K_disc*phi_neg_d2;
    const double theta = is_call ? theta_call : theta_put;

    const double rho_call = T*K_disc*Phi(d_2);
    const double rho_put  = -T*K_disc*Phi(-d_2);
    const double rho = is_call ? rho_call : rho_put;

    const double p = price(S,K,r,q,T,v,is_call);
    return { p, delta, gamma, vega, theta, rho };
}


} // namespace vol::bs
