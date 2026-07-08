add_library(ull_feeds STATIC
    feed_handler.cpp
    structural_scan.cpp
    data_access_view.cpp
)

target_include_directories(ull_feeds
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(ull_feeds
    PUBLIC
        ull_common
)

if(MSVC)
    target_compile_options(ull_feeds PRIVATE /arch:AVX2)
else()
    target_compile_options(ull_feeds PRIVATE -mavx2)
endif()
