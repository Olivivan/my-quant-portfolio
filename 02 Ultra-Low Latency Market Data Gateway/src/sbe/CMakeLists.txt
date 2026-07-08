add_library(ull_sbe INTERFACE)

target_include_directories(ull_sbe
    INTERFACE
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(ull_sbe
    INTERFACE
        ull_common
)
