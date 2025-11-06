#include "libvol/models/black_scholes.hpp"
#include <iostream>

int main(){
    double S = 100.0, K = 100.0, r = 0.05, q = 0.02, T = 1.5, vol = 0.2;
    bool is_call = true; 

    double price = vol::bs::price(S, K, r, q, T, vol, is_call);

    std::cout << "Call price: " << price << "\n";

    return 0;
}