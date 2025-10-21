#include "libvol/models/black_scholes.hpp"
#include "libvol/core/constants.hpp"
#include <cmath>
#include <algorithm>
#include <math.h>


namespace vol::bs {
static inline double expm1_safe(double x){ return (std::abs(x)<1e-6)? x + 0.5*x*x : std::expm1(x);}


double phi(double x){ return std::exp(-0.5*x*x)/SQRT2PI; }


static inline double erfc_inv(double x){ // not used, placeholder for later
    if (x <= 0.0) return INFINITY;
    if (x >= 2.0) return -INFINITY;

    double p = (x < 1.0) ? x : 2.0 - x;
    double t = std::sqrt(-2.0 * std::log(p / 2.0));

    double r = -0.70711 * ((2.30753 + t * 0.27061) /
                            (1.0 + t * (0.99229 + t * 0.04481)) - t);

    return (x < 1.0) ? r : -r;
}



double Phi(double x){ return 0.5*std::erfc(-x/std::sqrt(2.0)); }


static inline double d1(double S,double K,double r,double q,double T,double v){
return (std::log(S/K) + (r - q + 0.5*v*v)*T)/(v*std::sqrt(T));
}
static inline double d2(double d1,double v,double T){ return d1 - v*std::sqrt(T); }


static inline double forward(double S,double r,double q,double T){ return S*std::exp((r-q)*T); }


double price(double S,double K,double r,double q,double T,double v,bool is_call){
if(T<=0) {
double intrinsic = is_call? std::max(0.0,S-K) : std::max(0.0,K-S);
return intrinsic;
}
double F = forward(S,r,q,T);
double disc = std::exp(-r*T);
double d_1 = d1(S,K,r,q,T,v);
double d_2 = d2(d_1,v,T);
if(is_call) return disc*(F*Phi(d_1) - K*Phi(d_2));
return disc*(K*Phi(-d_2) - F*Phi(-d_1));
}


PriceGreeks price_greeks(double S,double K,double r,double q,double T,double v,bool is_call){
double F = forward(S,r,q,T);
double disc = std::exp(-r*T);
if(T<=0 || v<=0) {
double p = price(S,K,r,q,T,v,is_call);
return {p, (is_call? (S>K?1:0) : (S<K?-1:0)), 0, 0, 0, 0};
}
double sqT = std::sqrt(T);
double d_1 = d1(S,K,r,q,T,v);
double d_2 = d2(d_1,v,T);
double Nd1 = Phi(is_call? d_1 : d_1-1e-32); // same Nd1 for call/put, protective nudge
double nd1 = phi(d_1);
double call_delta = std::exp(-q*T)*Phi(d_1);
double put_delta = call_delta - std::exp(-q*T);
double delta = is_call? call_delta : put_delta;
double gamma = std::exp(-q*T)*nd1/(S*v*sqT);
double vega = S*std::exp(-q*T)*nd1*sqT; // per 1.0 vol
double theta_call = -0.5*S*std::exp(-q*T)*nd1*v/sqT + q*S*std::exp(-q*T)*Phi(d_1) - r*K*disc*Phi(d_2);
double theta_put = -0.5*S*std::exp(-q*T)*nd1*v/sqT - q*S*std::exp(-q*T)*Phi(-d_1) + r*K*disc*Phi(-d_2);
double theta = is_call? theta_call : theta_put;
double rho_call = T*K*disc*Phi(d_2);
double rho_put = -T*K*disc*Phi(-d_2);
double rho = is_call? rho_call : rho_put;
double p = price(S,K,r,q,T,v,is_call);
return {p, delta, gamma, vega, theta, rho};
}
}