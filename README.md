# C-vol-surface-calib-engine

Notebooks don't work, working on fix

**MVP targets**
- BS price within 1e-8 vs closed form (tests included)
- Greeks within 1e-5 vs finite difference (tests included)
- IV solver robust: Newton with Brent fallback
- MC with antithetic + placeholder control variate, CI reported
- Python bindings: `pip install .` (wheel later), `import volpy`


**Next sprints**
- SVI raw/JW with no‑arb checks + single‑expiry fit
- Heston CF pricing (Gauss–Laguerre), then calibration
- Calibration framework (global + local), parameter bounds/penalties
- RND extraction (Breeden–Litzenberger) + diagnostics

## Performance
**Black-Scholes Performance**
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

**Binomial Tree Performance**
```
| Steps | Single Price | Greeks |
|-------|--------------|--------|
| 50    | 1.4 μs       | 13 μs  |
| 100   | ~5 μs        | ~45 μs |
| 256   | 33 μs        | 302 μs |
| 512   | 133 μs       | 1.2 ms |
```
Complete Benchmark paste in benchmarks file

**Convergence to Black-Scholes:**
- 50 steps: 0.048 error
- 256 steps: 0.009 error (sub-penny accuracy)
- 1024 steps: 0.002 error
