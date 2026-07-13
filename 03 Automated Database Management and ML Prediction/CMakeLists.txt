cmake_minimum_required(VERSION 3.21)
project(QuantPipeline VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# vcpkg toolchain integration (override via -DCMAKE_TOOLCHAIN_FILE)
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "vcpkg toolchain")
endif()

# Parallel compilation for MSVC
if(MSVC)
    add_compile_options(/MP /W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O3 -march=native)
endif()

find_package(Arrow REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(TBB REQUIRED)
find_package(hiredis CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# ONNX Runtime and libpqxx are provided as prebuilt/system libraries to avoid
# long vcpkg source builds and spaces-in-path issues with several vcpkg ports.
set(ONNXRUNTIME_DIR "${CMAKE_SOURCE_DIR}/external/onnxruntime-1.23.2"
    CACHE PATH "ONNX Runtime prebuilt root")
find_library(ONNXRUNTIME_LIB onnxruntime PATHS "${ONNXRUNTIME_DIR}/lib" REQUIRED NO_DEFAULT_PATH)
find_file(ONNXRUNTIME_DLL onnxruntime.dll PATHS "${ONNXRUNTIME_DIR}/lib" REQUIRED NO_DEFAULT_PATH)

set(LIBPQXX_DIR "${CMAKE_SOURCE_DIR}/external/libpqxx-install"
    CACHE PATH "libpqxx install root")
find_package(PostgreSQL REQUIRED)
find_library(LIBPQXX_LIB pqxx PATHS "${LIBPQXX_DIR}/lib" REQUIRED NO_DEFAULT_PATH)
find_path(LIBPQXX_INCLUDE_DIR pqxx/pqxx PATHS "${LIBPQXX_DIR}/include" REQUIRED NO_DEFAULT_PATH)

add_subdirectory(src/cpp)

enable_testing()
add_subdirectory(tests)
