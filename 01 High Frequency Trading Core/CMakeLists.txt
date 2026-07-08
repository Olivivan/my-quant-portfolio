cmake_minimum_required(VERSION 3.16)
project(HFT_Core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

enable_testing()

add_subdirectory(lib/hft_core_lib)
add_subdirectory(lib/hft_render_lib)
add_subdirectory(src)
add_subdirectory(tests)
