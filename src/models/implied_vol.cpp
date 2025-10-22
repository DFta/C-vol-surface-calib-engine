#include "libvol/models/black_scholes.hpp"
#include "libvol/math/root_finders.hpp"
#include <algorithm>
#include <cmath>


namespace vol::bs {
static inline double bs_price_diff(double S,double K,double r,double q,double T,bool is_call,double vol, double target){
    return price(S,K,r,q,T,vol,is_call) - target;
}


IVResult implied_vol(double S,double K,double r,double q,double T,double target,bool is_call,
double init,double tol){
    using namespace vol::root;
    // Guard rails
    double intrinsic = is_call? std::max(0.0, S*std::exp(-q*T) - K*std::exp(-r*T))
    : std::max(0.0, K*std::exp(-r*T) - S*std::exp(-q*T));
    double max_price = is_call? S*std::exp(-q*T) : K*std::exp(-r*T);
    if(target<=intrinsic) return {0.0,0,0,true};
    if(target>=max_price) return {1e3,0,0,true};


    int newton_iters=0; bool newton_ok=false; double vol=std::max(1e-6, init);


    auto f_df = [&](double x, double& f, double& df){
    auto g = price_greeks(S,K,r,q,T,x,is_call);
    f = g.price - target;
    df = g.vega; // per 1.0 vol
    };


    auto nr = newton(f_df, vol, tol, 20);
    newton_iters = nr.iters; vol = std::abs(nr.x);
    newton_ok = nr.converged && std::isfinite(vol) && vol>0;


        if(!newton_ok){
        // Fallback to Brent on [lo, hi]
        // Wide bracket; adapt if price extremely high
        double lo=1e-8, hi=5.0;
        auto f = [&](double x){ return bs_price_diff(S,K,r,q,T,is_call,x,target); };
        // expand hi if needed
        while(f(lo)*f(hi) > 0 && hi < 50.0){ hi *= 1.5; }
        auto br = brent(f, lo, hi, tol, 100);
        return {br.x, newton_iters, br.iters, br.converged};
        }
    return {vol, newton_iters, 0, true};
    }
}