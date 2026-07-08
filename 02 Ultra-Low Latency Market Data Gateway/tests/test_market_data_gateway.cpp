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

TEST_CASE("MarketDataGateway routes message types via FNV-1a jump table", "[gateway][routing]") {
    ull::gateway::MarketDataGateway gateway;

    REQUIRE(gateway.route_message_type("QUOTE") == ull::gateway::MessageType::quote);
    REQUIRE(gateway.route_message_type("TRADE") == ull::gateway::MessageType::trade);
    REQUIRE(gateway.route_message_type("BOOK_UPDATE") == ull::gateway::MessageType::book_update);
    REQUIRE(gateway.route_message_type("HEARTBEAT") == ull::gateway::MessageType::heartbeat);
    REQUIRE(gateway.route_message_type("UNKNOWN") == ull::gateway::MessageType::unknown);
    REQUIRE(gateway.routed_message_count() == 5);
}

TEST_CASE("Compile-time hash constants match runtime hashing", "[gateway][routing]") {
    using ull::common::fnv1a_64;

    REQUIRE(fnv1a_64("QUOTE") == ull::gateway::MarketDataGateway::quote_hash);
    REQUIRE(fnv1a_64("TRADE") == ull::gateway::MarketDataGateway::trade_hash);
    REQUIRE(fnv1a_64("BOOK_UPDATE") == ull::gateway::MarketDataGateway::book_update_hash);
    REQUIRE(fnv1a_64("HEARTBEAT") == ull::gateway::MarketDataGateway::heartbeat_hash);
}

TEST_CASE("MarketDataGateway defers non-critical tasks off hot path", "[gateway][deferred]") {
    ull::gateway::MarketDataGateway gateway;

    REQUIRE(gateway.enqueue_deferred_task({ull::gateway::DeferredTaskKind::logging, 10}));
    REQUIRE(gateway.enqueue_deferred_task({ull::gateway::DeferredTaskKind::persistence, 20}));

    REQUIRE(gateway.process_deferred_tasks(1) == 1);
    REQUIRE(gateway.deferred_processed_count() == 1);
    REQUIRE(gateway.process_deferred_tasks(8) == 1);
    REQUIRE(gateway.deferred_processed_count() == 2);
    REQUIRE(gateway.process_deferred_tasks(8) == 0);
}
