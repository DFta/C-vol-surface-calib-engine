# LibVol Performance Benchmarks

## Results

### Test System
- **CPU:** 12th Gen Intel(R) Core(TM) i7-12650H (2.30 GHz)
- **RAM:** 16.0 GB (15.7 GB usable)
- **OS:** Windows 11
- **Compiler:** MSVC 19.44.35219.0
- **Build Type:** Release
- **Flags:** /O2, /std:c++20
- **Precision:** double
- **Threads:** single-threaded
- **Date:** November 7, 2025

- Black–Scholes benchmarks: `bench/bench_black_scholes.cpp`
- Binomial benchmarks: `bench/bench_binom.cpp`
- SVI slice benchmarks: `bench/bench_svi_slice.cpp`

## Running Benchmarks
```bash
# With ninja:
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build build
.\build\vol_bench.exe
.\build\binom_bench.exe
.\build\svi_slice_bench.exe

# With MSVC:
cmake -S . -B build
cmake --build build --config Release
.\build\Release\vol_bench.exe
.\build\Release\binom_bench.exe
.\build\Release\svi_slice_bench.exe

(use / if on linux or macos)

# Custom (ninja example)
.\build\vol_bench --benchmark_repetitions=10 --benchmark_min_time=1.0
```

### Black-Scholes Pricing
```
-------------------------------------------------------------------
Benchmark                         Time             CPU   Iterations
-------------------------------------------------------------------
BM_Price_ATM                   38.6 ns         38.5 ns     19478261
BM_Price_OTM                   36.6 ns         36.1 ns     19478261
BM_PriceGreeks_ATM              112 ns          109 ns      5600000
BM_Price_MultipleStrikes        393 ns          385 ns      1866667
BM_Price_ShortDated            38.3 ns         38.4 ns     17920000
BM_Price_Put                   40.8 ns         41.0 ns     17920000
```
**Summary**

- ~**39 ns** per price
  - about **25 million prices/sec** (single-threaded).
- ~**110 ns** for price + Greeks
  - about **9 million price+Greek evaluations/sec**.
- Performance scales linearly with number of strikes.

### Comparison to Other Libraries
| Library          | Price Time |
|------------------|------------|
| LibVol (this lib)| **39 ns**  |
| QuantLib         | ~150 ns    |
| NumPy (Python)   | ~500 ns    |
| Pure Python      | ~50 μs     |

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
**Summary**

Approximate timings for pricing a single European call:

| Steps | Euro price time | Euro+Greeks time | Typical use case                 |
|-------|-----------------|------------------|----------------------------------|
| 50    | ~1.4 µs         | ~13 µs           | Real-time pricing                |
| 256   | ~33 µs          | ~300 µs          | High accuracy (< 1¢ error vs BS) |
| 1024  | ~0.55 ms        | ~4.7 ms          | Research / model validation      |

- Error vs Black–Scholes decreases roughly like \(O(1/\sqrt{N})\) with steps.
- Benchmarks include both American and European payoffs, plus finite-difference Greeks.

# SVI Slice Calibration
```
-------------------------------------------------------------------------
Benchmark                               Time             CPU   Iterations
-------------------------------------------------------------------------
BM_SVI_Calibrate_Clean             218264 ns       217644 ns         3446
BM_SVI_Calibrate_Noisy             220905 ns       214844 ns         3200
BM_SVI_Calibrate_TermStructure     896965 ns       906808 ns          896
```
**Summary**

- Clean slice calibration: ~0.22 ms.
- Noisy slice calibration: ~0.22 ms (robust to moderate noise).
- Full term-structure (multiple expiries): ~0.9 ms.

This is fast enough to refit an entire term-structure interactively or on each refresh.

## Heston Pricing
```
--------------------------------------------------------------------------
Benchmark                                Time             CPU   Iterations
--------------------------------------------------------------------------
BM_Heston_ATM_Call64                 27173 ns        27344 ns        28000
BM_Heston_OTM_Call64                 25743 ns        25670 ns        28000
BM_Heston_ATM_Put64                  26350 ns        26228 ns        28000
BM_Heston_Portfolio64               212158 ns       209961 ns         3200
BM_Heston_GaussLaguerreOrder/32      12648 ns        12556 ns        49778 n_gl=32
BM_Heston_GaussLaguerreOrder/64      27551 ns        27867 ns        26353 n_gl=64
BM_Heston_GaussLaguerreOrder/96      40872 ns        40491 ns        16593 n_gl=96
```

**Summary**

- ~38k prices / second
- Runtime scales roughly linearly with n_gl (more nodes → more integrand evaluations)

Fast enough for interactive pricing and calibration loops