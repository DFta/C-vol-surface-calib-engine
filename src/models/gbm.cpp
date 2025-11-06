#include "libvol/mc/gbm.hpp"
#include "libvol/models/black_scholes.hpp"
#include "libvol/util/stats.hpp"
#include <random>
#include <cmath>
#include <vector>
#include <cstddef>
#include <algorithm> // for std::max

namespace vol::mc {

    MCResult european_vanilla_gbm(double S,double K,double r,double q,double T,double vol,bool is_call, 
                                std::uint64_t n_paths, std::uint64_t seed){
        
        if (n_paths % 2ULL) ++n_paths;

        std::mt19937_64 rng(seed);
        std::normal_distribution<double> Z(0.0,1.0);

        const double sqT = std::sqrt(T);
        const double mu = r - q - 0.5*vol*vol;
        const double disc = std::exp(-r*T);
        const double EX = S*std::exp(-q*T); 

        const std::uint64_t pairs = n_paths/2;

        std::vector<double> Y; Y.reserve(pairs);
        std::vector<double> X; X.reserve(pairs);

        for(std::uint64_t i=0; i<pairs; ++i){
            const double z  = Z(rng);
            const double ez = vol*sqT*z;

            const double STp = S*std::exp(mu*T + ez);
            const double STm = S*std::exp(mu*T - ez);

            const double payp = is_call ? std::max(0.0, STp-K) : std::max(0.0, K-STp);
            const double paym = is_call ? std::max(0.0, STm-K) : std::max(0.0, K-STm);

            const double payoff_avg = 0.5*(payp + paym);
            const double ST_avg = 0.5*(STp + STm);

            Y.push_back(disc * payoff_avg); // target variable
            X.push_back(disc * ST_avg);    
        }

        auto [mY, vY] = vol::stats::mean_var(Y);
        auto [mX, vX] = vol::stats::mean_var(X);

        // covariance
        double covYX = 0.0;
        for (std::size_t i=0; i<Y.size(); ++i) covYX += (Y[i]-mY)*(X[i]-mX);
        covYX /= (Y.size() > 1 ? (Y.size()-1) : 1);

        const double beta = (vX > 0.0 ? covYX / vX : 0.0);

        //control variate
        std::vector<double> Ytilde; Ytilde.reserve(Y.size());
        for (std::size_t i=0; i<Y.size(); ++i) {
            Ytilde.push_back( Y[i] - beta*(X[i] - EX) );
        }

        auto [m, v] = vol::stats::mean_var(Ytilde);
        const double se = std::sqrt(v / Ytilde.size());
        return {m, se, n_paths };
    }

}
