add_executable(hft_tests tests_main.cpp)
target_link_libraries(hft_tests PRIVATE hft_core_lib)

add_test(NAME hft_tests COMMAND hft_tests)
