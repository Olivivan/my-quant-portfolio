#include "ull/common/spsc_ring_buffer.hpp"

#include <catch2/catch.hpp>

TEST_CASE("SpscRingBuffer push and pop", "[common]") {
    ull::common::SpscRingBuffer<int, 8> buffer;

    REQUIRE(buffer.empty());
    REQUIRE(buffer.try_push(42));

    const auto value = buffer.try_pop();
    REQUIRE(value.has_value());
    REQUIRE(*value == 42);
    REQUIRE(buffer.empty());
}
