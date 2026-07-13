find_package(GTest REQUIRED)

add_executable(quant_pipeline_tests
    test_cleaner.cpp
    test_indicators.cpp
    test_parser.cpp
)

target_link_libraries(quant_pipeline_tests PRIVATE
    quant_pipeline_lib
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(quant_pipeline_tests)
