#include "ull/common/thread_topology.hpp"
#include "ull/gateway/market_data_gateway.hpp"

#include <catch2/catch.hpp>

#include <limits>

TEST_CASE("Thread pinning rejects invalid CPU index", "[common][topology]") {
    const bool pinned = ull::common::pin_current_thread_to_cpu((std::numeric_limits<std::size_t>::max)());
    REQUIRE_FALSE(pinned);
}

TEST_CASE("NUMA buffer allocation provides working memory", "[common][topology]") {
    auto buffer = ull::common::NumaBuffer::allocate(4096, 0);

    REQUIRE(buffer.valid());
    REQUIRE(buffer.size() >= 4096);

    auto view = buffer.bytes();
    REQUIRE(!view.empty());
    view[0] = std::byte{0x2A};
    REQUIRE(view[0] == std::byte{0x2A});
}

TEST_CASE("MarketDataGateway exposes pinning and NUMA working-set hooks", "[gateway][topology]") {
    ull::gateway::MarketDataGateway gateway;

    REQUIRE(gateway.allocate_numa_working_set(8192, 0));
    REQUIRE(gateway.numa_working_set_ready());
}
