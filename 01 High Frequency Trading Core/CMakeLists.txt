cmake_minimum_required(VERSION 3.16)
project(HFT_Core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(HFT_ENABLE_WARNINGS "Enable strict compiler warnings" ON)
option(HFT_ENABLE_CLANG_TIDY "Enable clang-tidy during compilation if available" OFF)

if(HFT_ENABLE_WARNINGS)
	if(MSVC)
		add_compile_options(/W4 /permissive-)
	else()
		add_compile_options(-Wall -Wextra -Wpedantic)
	endif()
endif()

if(HFT_ENABLE_CLANG_TIDY)
	find_program(CLANG_TIDY_EXE NAMES clang-tidy)
	if(CLANG_TIDY_EXE)
		set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};--warnings-as-errors=*")
	else()
		message(WARNING "HFT_ENABLE_CLANG_TIDY is ON but clang-tidy was not found")
	endif()
endif()

enable_testing()
option(HFT_BUILD_PYTHON_BRIDGE "Build Python bindings (pybind11 module)" OFF)

add_subdirectory(lib/hft_core_lib)
add_subdirectory(lib/hft_render_lib)
add_subdirectory(src)
add_subdirectory(tests)

if(HFT_BUILD_PYTHON_BRIDGE)
	add_subdirectory(python_bridge)
endif()
