#include "libvol/models/black_scholes.hpp"
#include <iostream>

int main(){
    double S = 100.0, K = 100.0, r = 0.05, q = 0.02, T = 1.5, vol = 0.2;

    double p_call = vol::bs::price(S, K, r, q, T, vol, true);
    double p_put = vol::bs::price(S,K,r,q,T,vol,false);

    std::cout << "Call price: " << p_call << "\n" << "Put price: " << p_put << "\n";

    return 0;
}