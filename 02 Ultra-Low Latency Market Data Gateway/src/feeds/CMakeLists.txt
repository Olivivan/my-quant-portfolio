add_library(ull_feeds STATIC
    feed_handler.cpp
)

target_include_directories(ull_feeds
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(ull_feeds
    PUBLIC
        ull_common
)
