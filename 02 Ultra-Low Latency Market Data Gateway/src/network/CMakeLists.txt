add_library(ull_network STATIC
    udp_receiver.cpp
)

target_include_directories(ull_network
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(ull_network
    PUBLIC
        ull_common
)
