#pragma once
#include <vector>
#include <functional>


namespace vol::calib {
struct LSQResult { std::vector<double> x; double rmse; int iters; bool converged; };

//placeholder signature
LSQResult lbfgsb(
const std::vector<double>& x0,
const std::vector<double>& lb,
const std::vector<double>& ub,
const std::function<void(const std::vector<double>&, double&, std::vector<double>&)>& f_grad,
int maxit=500, double tol=1e-8);
}