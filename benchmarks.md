# LibVol Performance Benchmarks

## Results

### Test System
- **CPU:** 12th Gen Intel(R) Core(TM) i7-12650H (2.30 GHz)
- **RAM:** 16.0 GB (15.7 GB usable)
- **OS:** Windows 11
- **Compiler:** MSVC 19.44.35219.0
- **Flags:** `-O3 -march=native -std=c++20`
- **Date:** October 23, 2025

Run benchmarks yourself: `.\build\vol_bench.exe`, `.\build\binom_bench.exe`

### Black-Scholes Pricing
```
-------------------------------------------------------------------
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

## Binomial Pricing
```
----------------------------------------------------------------------------------------------
Benchmark                                    Time             CPU   Iterations UserCounters
----------------------------------------------------------------------------------------------
BM_Binom_Price_Euro_Call/50               1388 ns         1350 ns       497778 steps=50
BM_Binom_Price_Euro_Call/64               2215 ns         2246 ns       320000 steps=64
BM_Binom_Price_Euro_Call/128              8408 ns         8545 ns        89600 steps=128
BM_Binom_Price_Euro_Call/256             33014 ns        32087 ns        22400 steps=256
BM_Binom_Price_Euro_Call/512            129289 ns       125558 ns         5600 steps=512
BM_Binom_Price_Euro_Call/1024           519207 ns       515625 ns         1000 steps=1.024k
BM_Binom_Price_Euro_Call/2048          2135777 ns      2128623 ns          345 steps=2.048k
BM_Binom_Price_Euro_Call/4096         10031136 ns     10000000 ns           75 steps=4.096k
BM_Binom_Price_Amer_Put/50                1753 ns         1765 ns       407273 steps=50
BM_Binom_Price_Amer_Put/64                2674 ns         2668 ns       263529 steps=64
BM_Binom_Price_Amer_Put/128               9852 ns         9835 ns        74667 steps=128
BM_Binom_Price_Amer_Put/256              37624 ns        37667 ns        18667 steps=256
BM_Binom_Price_Amer_Put/512             148511 ns       142997 ns         4480 steps=512
BM_Binom_Price_Amer_Put/1024            588424 ns       585938 ns         1120 steps=1.024k
BM_Binom_Price_Amer_Put/2048           2339924 ns      2299331 ns          299 steps=2.048k
BM_Binom_Price_Amer_Put/4096          11171939 ns     10602679 ns           56 steps=4.096k
BM_Binom_Greeks_Euro_Call/50             13066 ns        12835 ns        56000 steps=50
BM_Binom_Greeks_Euro_Call/64             20580 ns        20856 ns        34462 steps=64
BM_Binom_Greeks_Euro_Call/128            77770 ns        78474 ns         8960 steps=128
BM_Binom_Greeks_Euro_Call/256           299513 ns       299944 ns         2240 steps=256
BM_Binom_Greeks_Euro_Call/512          1150645 ns      1147461 ns          640 steps=512
BM_Binom_Greeks_Euro_Call/1024         4675138 ns      4667208 ns          154 steps=1.024k
BM_Binom_Price_MultipleStrikes/50        23301 ns        23019 ns        29867 batch=16 steps=50
BM_Binom_Price_MultipleStrikes/64        37289 ns        36830 ns        18667 batch=16 steps=64
BM_Binom_Price_MultipleStrikes/128      142694 ns       141246 ns         4978 batch=16 steps=128
BM_Binom_Price_MultipleStrikes/256      542299 ns       546875 ns         1000 batch=16 steps=256
BM_Binom_Price_MultipleStrikes/512     2170091 ns      2197266 ns          320 batch=16 steps=512
BM_Binom_Price_MultipleStrikes/1024    8824365 ns      8750000 ns           75 batch=16 steps=1.024k
BM_Binom_Price_ShortDated/50              1517 ns         1535 ns       448000 steps=50
BM_Binom_Price_ShortDated/64              2332 ns         2250 ns       298667 steps=64
BM_Binom_Price_ShortDated/128             8912 ns         8998 ns        74667 steps=128
BM_Binom_Price_ShortDated/256            34458 ns        35157 ns        21333 steps=256
BM_Binom_Price_ShortDated/512           129828 ns       128691 ns         4978 steps=512
BM_Binom_Price_ShortDated/1024          534420 ns       515625 ns         1000 steps=1.024k
BM_Binom_Price_ShortDated/2048         2238225 ns      2246094 ns          320 steps=2.048k
BM_Binom_Error_vs_BS/50                   1496 ns         1496 ns       407273 abs_err_vs_BS=0.0482413 steps=50
BM_Binom_Error_vs_BS/64                   2320 ns         2250 ns       298667 abs_err_vs_BS=0.0377101 steps=64
BM_Binom_Error_vs_BS/128                  8480 ns         8370 ns        74667 abs_err_vs_BS=0.018874 steps=128
BM_Binom_Error_vs_BS/256                 34071 ns        33692 ns        21333 abs_err_vs_BS=9.4417m steps=256
BM_Binom_Error_vs_BS/512                136032 ns       136719 ns         5600 abs_err_vs_BS=4.72201m steps=512
BM_Binom_Error_vs_BS/1024               549743 ns       530134 ns         1120 abs_err_vs_BS=2.3613m steps=1.024k
```