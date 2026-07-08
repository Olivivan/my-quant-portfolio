add_library(ull_common STATIC
    thread_topology.cpp
)

target_include_directories(ull_common
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)
