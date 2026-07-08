cmake_minimum_required(VERSION 3.16)
project(HFT_Core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(HFT_ENABLE_WARNINGS "Enable strict compiler warnings" ON)
option(HFT_ENABLE_CLANG_TIDY "Enable clang-tidy during compilation if available" OFF)
option(HFT_BUILD_DOCS "Enable Doxygen documentation target" ON)
option(HFT_DOCS_WITH_DOT "Enable Graphviz-backed diagrams for docs when dot is available" ON)
option(HFT_DOCS_WARN_AS_ERROR "Treat Doxygen warnings as build errors" OFF)

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

if(HFT_BUILD_DOCS)
	find_package(Doxygen QUIET)
	if(DOXYGEN_FOUND)
		set(HFT_DOXYGEN_HAVE_DOT NO)
		if(HFT_DOCS_WITH_DOT)
			find_program(HFT_DOT_EXECUTABLE NAMES dot dot.exe)
			if(HFT_DOT_EXECUTABLE)
				execute_process(
					COMMAND ${HFT_DOT_EXECUTABLE} -V
					RESULT_VARIABLE HFT_DOT_HEALTH_RESULT
					OUTPUT_QUIET
					ERROR_QUIET
				)
				if(HFT_DOT_HEALTH_RESULT EQUAL 0)
					set(HFT_DOXYGEN_HAVE_DOT YES)
				else()
					message(STATUS "dot executable found but failed health check; disabling Graphviz docs diagrams.")
				endif()
			else()
				message(STATUS "dot executable not found; disabling Graphviz docs diagrams.")
			endif()
		endif()

		if(HFT_DOCS_WARN_AS_ERROR)
			set(HFT_DOXYGEN_WARN_AS_ERROR YES)
		else()
			set(HFT_DOXYGEN_WARN_AS_ERROR NO)
		endif()

		set(HFT_DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
		set(HFT_DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

		configure_file(${HFT_DOXYGEN_IN} ${HFT_DOXYGEN_OUT} @ONLY)

		add_custom_target(docs
			COMMAND ${DOXYGEN_EXECUTABLE} ${HFT_DOXYGEN_OUT}
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			COMMENT "Generating API documentation with Doxygen"
			VERBATIM
		)
	else()
		message(STATUS "Doxygen not found; skipping docs target. Install Doxygen or set HFT_BUILD_DOCS=OFF.")
	endif()
endif()
