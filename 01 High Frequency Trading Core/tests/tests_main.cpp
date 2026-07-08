#include <catch2/catch_test_macros.hpp>
#include "../lib/hft_core_lib/hft_core.hpp"

TEST_CASE("Price-Time Priority Logic", "[core][matching]") {
    SECTION("Price Priority: Higher Buy Price Fills First") {
        HFT::HFTCore engine;

        // Place two passive buy orders at different prices
        REQUIRE(engine.LimitOrder({1, 100.0, 10, true, 1000})); // Order A: $100.0
        REQUIRE(engine.LimitOrder({2, 101.0, 10, true, 1001})); // Order B: $101.0 (Better Price)

        // Place one aggressive sell order that can only fill 10 units
        REQUIRE(engine.LimitOrder({3, 100.0, 10, false, 1002}));

        // The better price ($101.0) must be filled first despite arriving later
        REQUIRE(engine.GetBuyBook().find(101.0) == engine.GetBuyBook().end());
        REQUIRE(engine.GetBuyBook().find(100.0) != engine.GetBuyBook().end());
        REQUIRE(engine.GetBuyBook().at(100.0).size() == 1);
    }

    SECTION("Time Priority: Older Order Fills First at Same Price") {
        HFT::HFTCore engine;

        // Place two buy orders at the same price with different timestamps
        REQUIRE(engine.LimitOrder({1, 100.0, 10, true, 1000})); // Order A: Earlier
        REQUIRE(engine.LimitOrder({2, 100.0, 10, true, 1001})); // Order B: Later

        // Place aggressive sell order for 10 units
        REQUIRE(engine.LimitOrder({3, 100.0, 10, false, 1002}));

        // The earlier order (Order A) must be filled
        auto& ordersAtPrice = engine.GetBuyBook().at(100.0);
        REQUIRE(ordersAtPrice.front().id == 2); // Only Order B remains
    }

    SECTION("Partial Fills and Remainder Persistence") {
        HFT::HFTCore engine;

        // Large Buy Order
        REQUIRE(engine.LimitOrder({1, 100.0, 100, true, 1000}));
        
        // Smaller Sell Order
        REQUIRE(engine.LimitOrder({2, 100.0, 30, false, 1001}));

        // Order 1 should have 70 units remaining in the book
        auto& ordersAtPrice = engine.GetBuyBook().at(100.0);
        REQUIRE(ordersAtPrice.front().quantity == 70);
    }

    SECTION("Timestamp Priority Works Even If Arrival Is Out Of Order") {
        HFT::HFTCore engine;

        // Insert newer order first, then older one at same price.
        REQUIRE(engine.LimitOrder({2, 100.0, 10, true, 2000})); // Newer timestamp
        REQUIRE(engine.LimitOrder({1, 100.0, 10, true, 1000})); // Older timestamp

        // Aggressive sell should match the older timestamp first.
        REQUIRE(engine.LimitOrder({3, 100.0, 10, false, 3000}));

        auto& ordersAtPrice = engine.GetBuyBook().at(100.0);
        REQUIRE(ordersAtPrice.front().id == 2);
    }

    SECTION("Equal Timestamp Preserves Arrival Order") {
        HFT::HFTCore engine;

        REQUIRE(engine.LimitOrder({1, 100.0, 10, true, 1000}));
        REQUIRE(engine.LimitOrder({2, 100.0, 10, true, 1000}));
        REQUIRE(engine.LimitOrder({3, 100.0, 10, false, 1001}));

        auto& ordersAtPrice = engine.GetBuyBook().at(100.0);
        REQUIRE(ordersAtPrice.front().id == 2);
    }
}

TEST_CASE("Order Management Operations", "[core][management]") {
    SECTION("Cancel removes resting order") {
        HFT::HFTCore engine;
        REQUIRE(engine.LimitOrder({10, 99.0, 5, true, 1000}));

        REQUIRE(engine.CancelOrder(10));
        REQUIRE_FALSE(engine.CancelOrder(10));
        REQUIRE(engine.GetBuyBook().empty());
    }

    SECTION("Replace updates order and keeps id") {
        HFT::HFTCore engine;
        REQUIRE(engine.LimitOrder({20, 100.0, 5, true, 1000}));

        REQUIRE(engine.ReplaceOrder(20, 101.0, 7, 1001));
        REQUIRE(engine.GetBuyBook().find(100.0) == engine.GetBuyBook().end());
        REQUIRE(engine.GetBuyBook().find(101.0) != engine.GetBuyBook().end());
        REQUIRE(engine.GetBuyBook().at(101.0).front().id == 20);
        REQUIRE(engine.GetBuyBook().at(101.0).front().quantity == 7);
    }

    SECTION("Replace can trigger matching when crossing") {
        HFT::HFTCore engine;
        REQUIRE(engine.LimitOrder({30, 100.0, 5, false, 1000}));
        REQUIRE(engine.LimitOrder({31, 99.0, 5, true, 1001}));

        REQUIRE(engine.ReplaceOrder(31, 100.0, 5, 1002));
        REQUIRE(engine.GetBuyBook().empty());
        REQUIRE(engine.GetSellBook().empty());
    }

    SECTION("Invalid replacement payload does not mutate order") {
        HFT::HFTCore engine;
        REQUIRE(engine.LimitOrder({40, 100.0, 9, true, 1000}));

        REQUIRE_FALSE(engine.ReplaceOrder(40, -1.0, 9, 1001));
        REQUIRE(engine.GetBuyBook().find(100.0) != engine.GetBuyBook().end());
        REQUIRE(engine.GetBuyBook().at(100.0).front().quantity == 9);
        REQUIRE(engine.GetBuyBook().at(100.0).front().id == 40);
    }
}

TEST_CASE("Input Validation", "[core][validation]") {
    HFT::HFTCore engine;

    SECTION("Rejects zero quantity") {
        REQUIRE_FALSE(engine.LimitOrder({50, 100.0, 0, true, 1000}));
    }

    SECTION("Rejects non-positive price") {
        REQUIRE_FALSE(engine.LimitOrder({51, 0.0, 1, true, 1000}));
        REQUIRE_FALSE(engine.LimitOrder({52, -100.0, 1, true, 1000}));
    }

    SECTION("Rejects duplicate active order id") {
        REQUIRE(engine.LimitOrder({53, 100.0, 1, true, 1000}));
        REQUIRE_FALSE(engine.LimitOrder({53, 101.0, 1, true, 1001}));
    }

    SECTION("Rejects negative timestamp") {
        REQUIRE_FALSE(engine.LimitOrder({54, 100.0, 1, true, -1}));
    }
}

TEST_CASE("Crossed Book Behavior", "[core][cross]") {
    HFT::HFTCore engine;

    SECTION("Aggressive buy sweeps best asks in ascending price order") {
        REQUIRE(engine.LimitOrder({60, 100.10, 4, false, 1000}));
        REQUIRE(engine.LimitOrder({61, 100.20, 4, false, 1001}));
        REQUIRE(engine.LimitOrder({62, 100.30, 4, false, 1002}));

        REQUIRE(engine.LimitOrder({63, 100.20, 9, true, 1003}));

        REQUIRE(engine.GetSellBook().find(100.10) == engine.GetSellBook().end());
        REQUIRE(engine.GetSellBook().find(100.20) == engine.GetSellBook().end());
        REQUIRE(engine.GetSellBook().find(100.30) != engine.GetSellBook().end());
        REQUIRE(engine.GetSellBook().at(100.30).front().quantity == 3);
    }
}