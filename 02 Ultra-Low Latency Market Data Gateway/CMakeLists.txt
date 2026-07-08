cmake_minimum_required(VERSION 3.23)

project(ultra_low_latency_gateway VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/CompilerOptions.cmake)
include(cmake/Dependencies.cmake)

enable_testing()

add_subdirectory(src/common)
add_subdirectory(src/network)
add_subdirectory(src/feeds)
add_subdirectory(src/gateway)
add_subdirectory(apps/gateway)
add_subdirectory(tests)
