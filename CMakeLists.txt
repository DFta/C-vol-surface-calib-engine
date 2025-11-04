cmake_minimum_required(VERSION 3.20)
project(libvol LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)
find_package(Threads REQUIRED)

# Dependencies
FetchContent_Declare(pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v2.12.0)
FetchContent_Declare(catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.4)
FetchContent_Declare(benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.4)
set(BENCHMARK_ENABLE_TESTING Off CACHE BOOL "" FORCE)

# Google Benchmark config (avoid -Werror on MinGW)
set(BENCHMARK_ENABLE_WERROR  Off CACHE BOOL "" FORCE)

# If caller passes -DPYTHON_EXECUTABLE=... use it for pybind11
if(DEFINED PYTHON_EXECUTABLE)
    set(PYBIND11_PYTHON_EXECUTABLE "${PYTHON_EXECUTABLE}" CACHE FILEPATH "" FORCE)
endif()


FetchContent_MakeAvailable(pybind11 catch2 benchmark)

# Library
add_library(vol STATIC
    src/models/black_scholes.cpp
    src/models/implied_vol.cpp
    src/models/gbm.cpp
    src/models/binom.cpp
    src/models/svi.cpp
    src/calib/svi_slice.cpp
    src/calib/least_squares.cpp
    )
target_include_directories(vol PUBLIC include)

# Require C++20 at target level (good with IDEs)
target_compile_features(vol PUBLIC cxx_std_20)

if (MSVC)
    target_compile_options(vol PRIVATE /W4 /permissive- /EHsc /bigobj)
    target_compile_definitions(vol PRIVATE NOMINMAX _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(vol PRIVATE -Wall -Wextra -Wpedantic -Werror=return-type)
endif()

# Python bindings
pybind11_add_module(volpy bindings/python_bindings.cpp)
target_link_libraries(volpy PRIVATE vol)

# Tests
enable_testing()
add_executable(vol_tests
    tests/test_black_scholes.cpp
    tests/test_implied_vol.cpp
    tests/test_binom.cpp
    )
target_link_libraries(vol_tests PRIVATE vol Catch2::Catch2WithMain)
add_test(NAME vol_tests COMMAND vol_tests)

# SVI example / test
add_executable(svi_slice_demo examples/svi_slice_demo.cpp)
target_include_directories(svi_slice_demo PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(svi_slice_demo PRIVATE vol)  # or whatever your core lib target is


# Benchmarks
add_executable(vol_bench bench/bench_black_scholes.cpp)
target_link_libraries(vol_bench PRIVATE vol benchmark::benchmark Threads::Threads)
add_executable(binom_bench bench/bench_binom.cpp)
target_link_libraries(binom_bench PRIVATE vol benchmark::benchmark Threads::Threads)

