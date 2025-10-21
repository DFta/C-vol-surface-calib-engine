#include <benchmark/benchmark.h>
#include "libvol/models/black_scholes.hpp"


static void BM_BS1000(benchmark::State& state){
int n=1000; for(auto _: state){
double acc=0; for(int i=0;i<n;++i){
acc += vol::bs::price(100, 50 + i*0.1, 0.02, 0.0, 1.0, 0.2, true);
}
benchmark::DoNotOptimize(acc);
}
}
BENCHMARK(BM_BS1000);
BENCHMARK_MAIN();