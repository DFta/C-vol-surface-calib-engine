#pragma once
#include <vector>


namespace vol::heston {
struct Params { double kappa, theta, sigma, rho, v0; };

//to be implemented
double price_cf(double S,double K,double r,double q,double T,const Params& p,bool is_call,int n_gl=64);
}