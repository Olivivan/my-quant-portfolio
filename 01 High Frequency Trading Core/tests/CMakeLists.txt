include(FetchContent)

FetchContent_Declare(
	Catch2
	GIT_REPOSITORY https://github.com/catchorg/Catch2.git
	GIT_TAG v3.6.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(hft_tests tests_main.cpp)
target_link_libraries(hft_tests PRIVATE hft_core_lib Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(hft_tests)
