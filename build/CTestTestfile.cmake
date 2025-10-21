# CMake generated Testfile for 
# Source directory: C:/Users/raphe/Documents/GitHub/C-vol-surface-calib-engine
# Build directory: C:/Users/raphe/Documents/GitHub/C-vol-surface-calib-engine/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[vol_tests]=] "C:/Users/raphe/Documents/GitHub/C-vol-surface-calib-engine/build/vol_tests.exe")
set_tests_properties([=[vol_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/raphe/Documents/GitHub/C-vol-surface-calib-engine/CMakeLists.txt;50;add_test;C:/Users/raphe/Documents/GitHub/C-vol-surface-calib-engine/CMakeLists.txt;0;")
subdirs("_deps/pybind11-build")
subdirs("_deps/catch2-build")
subdirs("_deps/benchmark-build")
