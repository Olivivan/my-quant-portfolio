add_library(ull_network STATIC
    udp_receiver.cpp
    kernel_bypass.cpp
)

target_include_directories(ull_network
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(ull_network
    PUBLIC
        ull_common
)

if(ULL_ENABLE_EFVI)
    target_compile_definitions(ull_network PUBLIC ULL_ENABLE_EFVI=1)
else()
    target_compile_definitions(ull_network PUBLIC ULL_ENABLE_EFVI=0)
endif()

if(UNIX AND NOT APPLE)
    target_compile_definitions(ull_network PUBLIC ULL_PLATFORM_LINUX=1)
else()
    target_compile_definitions(ull_network PUBLIC ULL_PLATFORM_LINUX=0)
endif()
