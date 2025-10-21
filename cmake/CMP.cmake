cmake_minimum_required(VERSION 3.20)
project(libvol LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


include(cmake/CPM.cmake)


CPMAddPackage("gh:pybind/pybind11@2.12.0")
CPMAddPackage("gh:catchorg/Catch2@3.5.4")
CPMAddPackage(NAME benchmark GITHUB_REPOSITORY google/benchmark VERSION 1.8.4 OPTIONS "BENCHMARK_ENABLE_TESTING Off")


add_library(vol STATIC
src/models/black_scholes.cpp
src/models/implied_vol.cpp
src/mc/gbm.cpp
src/util/stats.cpp)


target_include_directories(vol PUBLIC include)


# Warnings
if (MSVC)
target_compile_options(vol PRIVATE /W4 /permissive-)
else()
target_compile_options(vol PRIVATE -Wall -Wextra -Wpedantic -Werror=return-type)
endif()


# Python bindings
pybind11_add_module(volpy bindings/python_bindings.cpp)
target_link_libraries(volpy PRIVATE vol)


# Tests
add_executable(vol_tests tests/test_black_scholes.cpp tests/test_implied_vol.cpp)
target_link_libraries(vol_tests PRIVATE vol Catch2::Catch2WithMain)
add_test(NAME vol_tests COMMAND vol_tests)


# Benchmarks
add_executable(vol_bench bench/bench_bs.cpp)
target_link_libraries(vol_bench PRIVATE vol benchmark::benchmark)


enable_testing()