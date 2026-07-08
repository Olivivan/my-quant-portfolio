add_executable(ull_benchmarks
    benchmark_latency.cpp
)

target_link_libraries(ull_benchmarks
    PRIVATE
        ull_gateway
        ull_feeds
        benchmark::benchmark
        benchmark::benchmark_main
)
