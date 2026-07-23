cmake_minimum_required(VERSION 3.18)
project(native_stat_arb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/O2 /openmp /EHsc)
else()
    add_compile_options(-O3 -fopenmp -march=native -ffast-math -pthread)
endif()

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

execute_process(
    COMMAND "${Python3_EXECUTABLE}" -c "import sysconfig; print(sysconfig.get_path('include'))"
    OUTPUT_VARIABLE PYTHON_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND "${Python3_EXECUTABLE}" -c "import pybind11; print(pybind11.get_include())"
    OUTPUT_VARIABLE PYBIND11_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_library(native_stat_arb SHARED
    src/stat_arb_engine.cpp 
    src/order_execution.cpp
)

# CRITICAL FIX: Explicitly include both the parent directory AND the inner directory 
# to make sure the compiler finds <Eigen/Dense> no matter how it was unzipped.
target_include_directories(native_stat_arb PRIVATE 
    include
    include/Eigen
    ${PYTHON_INCLUDE_DIR}
    ${PYBIND11_INCLUDE_DIR}
)

target_link_libraries(native_stat_arb PRIVATE Python3::Module)

# Explicitly link the Windows Sockets system library needed for network functions like inet_pton
if(MSVC)
    target_link_libraries(native_stat_arb PRIVATE Ws2_32.lib)
endif()

set_target_properties(native_stat_arb PROPERTIES
    PREFIX ""
    SUFFIX ".pyd"
)

find_package(OpenMP REQUIRED)
target_link_libraries(native_stat_arb PRIVATE OpenMP::OpenMP_CXX)
