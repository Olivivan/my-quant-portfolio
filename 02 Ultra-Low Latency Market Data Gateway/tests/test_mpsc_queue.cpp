#include "ull/common/mpsc_queue.hpp"

#include <catch2/catch.hpp>

#include <atomic>
#include <thread>
#include <vector>

TEST_CASE("MpscQueue supports multi-producer single-consumer flow", "[common][mpsc]") {
    constexpr std::size_t per_producer = 128;
    ull::common::MpscQueue<int, 512> queue;

    std::atomic<bool> start{false};
    std::thread p1([&]() {
        while (!start.load(std::memory_order_acquire)) {
        }
        for (std::size_t i = 0; i < per_producer; ++i) {
            while (!queue.try_enqueue(1000 + static_cast<int>(i))) {
            }
        }
    });

    std::thread p2([&]() {
        while (!start.load(std::memory_order_acquire)) {
        }
        for (std::size_t i = 0; i < per_producer; ++i) {
            while (!queue.try_enqueue(2000 + static_cast<int>(i))) {
            }
        }
    });

    start.store(true, std::memory_order_release);

    std::size_t consumed = 0;
    while (consumed < per_producer * 2) {
        const auto value = queue.try_dequeue();
        if (value.has_value()) {
            ++consumed;
        }
    }

    p1.join();
    p2.join();

    REQUIRE(consumed == per_producer * 2);
    REQUIRE_FALSE(queue.try_dequeue().has_value());
}

TEST_CASE("MpscQueue returns false when full", "[common][mpsc]") {
    ull::common::MpscQueue<int, 2> queue;

    REQUIRE(queue.try_enqueue(1));
    REQUIRE(queue.try_enqueue(2));
    REQUIRE_FALSE(queue.try_enqueue(3));
}
