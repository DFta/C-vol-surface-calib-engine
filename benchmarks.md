# LibVol Performance Benchmarks

## Results

### Test System
- **CPU:** 12th Gen Intel(R) Core(TM) i7-12650H (2.30 GHz)
- **RAM:** 16.0 GB (15.7 GB usable)
- **OS:** Windows 11
- **Compiler:** MSVC 19.44.35219.0
- **Flags:** `-O3 -march=native -std=c++20`
- **Date:** October 23, 2025


### Black-Scholes Pricing
```
Benchmark                         Time             CPU   Iterations
-------------------------------------------------------------------
BM_Price_ATM                   41.2 ns         41.9 ns     17920000
BM_Price_OTM                   39.0 ns         37.7 ns     18666667
BM_PriceGreeks_ATM              121 ns          120 ns      6400000
BM_Price_MultipleStrikes        423 ns          417 ns      1947826
BM_Price_ShortDated            42.4 ns         42.0 ns     16000000
BM_Price_Put                   44.9 ns         43.0 ns     16000000
```

### Interpretation

- **Single price calculation:** ~40 nanoseconds
  - ----> Can price **25 million options per second** (single-threaded) <----
  
- **Price + Greeks:** ~120 nanoseconds
  - 3x overhead for computing all Greeks (delta, gamma, vega, theta, rho) (low, other lesser libraries )
  - ----> Can compute **8 million Greeks per second** <----

- **Linear scaling:** Multiple strikes scale linearly (~42 ns per option)

### Comparison to Other Libraries

| Library          | Price Time |
|------------------|------------|
| LibVol (this lib)| **40 ns**  |
| QuantLib         | ~150 ns    |
| NumPy (Python)   | ~500 ns    |
| Pure Python      | ~50 μs     |

## Running Benchmarks
```bash
# Build with optimizations
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Run benchmarks
./build/vol_bench

# Run with custom iterations
./build/vol_bench --benchmark_repetitions=10 --benchmark_min_time=1.0
```

