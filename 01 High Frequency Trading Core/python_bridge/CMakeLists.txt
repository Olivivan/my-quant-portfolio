find_package(Python3 COMPONENTS Interpreter Development.Module REQUIRED)

execute_process(
    COMMAND ${Python3_EXECUTABLE} -m pybind11 --cmakedir
    RESULT_VARIABLE PYBIND11_CMD_RESULT
    OUTPUT_VARIABLE PYBIND11_CMAKE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT PYBIND11_CMD_RESULT EQUAL 0)
    message(FATAL_ERROR "pybind11 is required for Python bridge. Install with: python -m pip install pybind11")
endif()

list(APPEND CMAKE_PREFIX_PATH "${PYBIND11_CMAKE_DIR}")
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(hft_core_py MODULE hft_core_pybind.cpp)
target_link_libraries(hft_core_py PRIVATE hft_core_lib)

# Keep output under build/bin for consistency with other targets.
set_target_properties(hft_core_py PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
