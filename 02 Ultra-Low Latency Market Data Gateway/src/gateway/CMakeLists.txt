add_library(ull_gateway STATIC
    market_data_gateway.cpp
)

target_include_directories(ull_gateway
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(ull_gateway
    PUBLIC
        ull_common
        ull_network
        ull_feeds
    ull_sbe
)
