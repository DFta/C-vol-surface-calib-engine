#include "libvol/mc/gbm.hpp"
#include "libvol/models/black_scholes.hpp"
#include "libvol/util/stats.hpp"
#include <random>
#include <cmath>


namespace vol::mc {
MCResult european_vanilla_gbm(double S,double K,double r,double q,double T,double vol,bool is_call,
std::uint64_t n_paths, std::uint64_t seed){
std::mt19937_64 rng(seed);
std::normal_distribution<double> Z(0.0,1.0);
const double dt=T; const double mu = r - q - 0.5*vol*vol; const double disc = std::exp(-r*T);
std::vector<double> samples; samples.reserve(n_paths/2);


auto bs = vol::bs::price(S,K,r,q,T,vol,is_call);


for(std::uint64_t i=0;i<n_paths/2;++i){
double z = Z(rng);
double sp = S*std::exp(mu*dt + vol*std::sqrt(dt)* z);
double sm = S*std::exp(mu*dt + vol*std::sqrt(dt)*(-z));
double payp = is_call? std::max(0.0, sp-K) : std::max(0.0, K-sp);
double paym = is_call? std::max(0.0, sm-K) : std::max(0.0, K-sm);
double payoff = 0.5*(payp + paym);
// Control variate: subtract (simulated call - analytic call) to reduce variance
double cv = payoff - (is_call? std::max(0.0, S*std::exp(-q*T)*std::exp(vol*std::sqrt(T)*z) - K) : payoff); // simple placeholder
samples.push_back(disc*(payoff) - (disc*cv - bs));
}
auto [m, v] = vol::stats::mean_var(samples);
return {m, std::sqrt(v/samples.size()), n_paths};
}
}