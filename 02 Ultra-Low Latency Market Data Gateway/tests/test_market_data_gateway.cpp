#include "ull/gateway/market_data_gateway.hpp"

#include <catch2/catch.hpp>

TEST_CASE("MarketDataGateway lifecycle", "[gateway]") {
    ull::gateway::MarketDataGateway gateway;

    REQUIRE_FALSE(gateway.is_running());
    gateway.start();
    REQUIRE(gateway.is_running());
    gateway.stop();
    REQUIRE_FALSE(gateway.is_running());
}
