#pragma once
#include <tuple>

namespace vol::binom {

    struct PriceGreeks {
        double price, delta, gamma, vega, theta, rho; 
    };

    PriceGreeks price_greeks(double S, double K, double r, double q, double T, double vol, int steps, bool is_call, bool is_american);

    enum class TreeType {
        CRR,
        JR,
        EQP,
        TIAN
    }; // only CRR for now but inshallah later I will get to the rest

    double price(double S, double K, double r, double q, double T, double vol, int steps, bool is_call, bool is_american);

    struct BinomialResult {
        double price;
        int early_exercise_step;
    };

    BinomialResult price_w_info(double S, double K, double r, double q, double T, double vol, int steps, bool is_call, bool is_american);

} // namespace vol::binom
