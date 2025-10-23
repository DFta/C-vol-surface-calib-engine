# C-vol-surface-calib-engine

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