# C-vol-surface-calib-engine

**MVP targets**
- BS price within 1e-8 vs closed form (tests included)
- Greeks within 1e-5 vs finite difference (tests included)
- IV solver robust: Newton with Brent fallback
- Slice-by-slice SVI smile calibration (raw SVI per expiry) on top of BS IVs
- MC with antithetic + placeholder control variate, CI reported
- Python bindings: `pip install .` (wheel later), `import volpy`


**Next sprints**
- Heston CF pricing (Gauss–Laguerre), then calibration
- Calibration framework (global + local), parameter bounds/penalties
- RND extraction (Breeden–Litzenberger) + diagnostics

## SVI Slice-by-Slice Calibration (New)

**What it does**

- Takes a **single expiry** option strip (same \(T\), varying \(K\))  
- Computes **implied vols** via a robust BS IV solver (safeguarded Newton + Brent)  
- Converts to **log-moneyness** \(k = \ln(K/F)\) and **total variance** \(w = \sigma^2 T\)  
- Fits a **raw SVI** smile per expiry
- Uses **vega-weighted least squares** with gentle wing down-weighting for stability  
- Enforces basic no-arb sanity: \(b > 0\), \(|\rho| < 1\), \(\sigma > 0\) (soft penalties + box constraints)

**Pipeline**

1. Market prices → BS IV (`vol::bs::implied_vol`)  
2. IVs → total variance grid `(k_i, w_i)`  
3. Optimize raw SVI params `{a, b, ρ, m, σ}` per slice  
4. Use `total_variance(k, params)` + \(w/T\) to recover model IVs for pricing / plotting

**Performance**

## Svi Slice Performance
```
-------------------------------------------------------------------------
Benchmark                               Time             CPU   Iterations
-------------------------------------------------------------------------
BM_SVI_Calibrate_Clean             218825 ns       219727 ns         3200
BM_SVI_Calibrate_Noisy             224093 ns       219702 ns         2987
BM_SVI_Calibrate_TermStructure     923726 ns       934710 ns         1120
```

## Black-Scholes Performance
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

### Binomial Tree Performance
```
| Steps | Single Price  | Greeks  | Use Case                  |
|-------|---------------|---------|---------------------------|
| 50    | 1.4 μs        | 13 μs   | Real-time pricing         |
| 100   | ~5 μs         | ~45 μs  | Standard accuracy         |
| 256   | 33 μs         | 302 μs  | High accuracy (<1¢ error) |
| 512   | 133 μs        | 1.2 ms  | Research/validation       |
```

**Convergence to Black-Scholes:**
- 50 steps: 0.048 error
- 256 steps: 0.009 error
- 1024 steps: 0.002 error
