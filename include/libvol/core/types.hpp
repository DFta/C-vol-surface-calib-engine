#pragma once

//had more but deleted the real(double) and vec(vector) types that chatgpt recommended since I didn't use them
//doesn't really make sense to move optionspec to another header either
//(plain vanilla euro option specification)
namespace vol {
struct OptionSpec { double S, K, r, q, T; bool is_call; };
}