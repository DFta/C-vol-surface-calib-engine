#pragma once
#include <vector>
#include <cstdint>


namespace vol {
using Real = double;
using Vec = std::vector<Real>;
struct OptionSpec { Real S, K, r, q, T; bool is_call; };
}