add_executable(ull_tests
    test_main.cpp
    test_spsc_ring_buffer.cpp
    test_market_data_gateway.cpp
)

target_link_libraries(ull_tests
    PRIVATE
        ull_gateway
    Catch2::Catch2
)

include(Catch)
catch_discover_tests(ull_tests)
