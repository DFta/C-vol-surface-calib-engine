# C-vol-surface-calib-engine

Notebooks don't work, working on fix

Black-Scholes pricing with 1e-8 precision
Binomial pricing for American + Euro options
Implied volume engine
Python wrappers (used to be able to use them in the notebooks, maybe you can get them to work. Idk what caused it exactly but as mentioned i'm working on a fix)

## Building from Source

### Prerequisites

#### Required
- **C++ Compiler** with C++20 support:
  - GCC 11+ (Linux)
  - Clang 13+ (macOS/Linux)
  - MSVC 2019+ (Windows)
- **CMake** 3.20 or higher
- **Git**

#### Optional (for Python bindings)
- **Python** 3.8 or higher
- **pip** (Python package manager)

### Quick Start
```bash
git clone https://github.com/DFta/libvol.git
cd libvol

# Build (Release mode for best performance)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Run tests (optional but recommended)
./build/vol_tests

# Run benchmarks (see Benchmarks.md)
```
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
