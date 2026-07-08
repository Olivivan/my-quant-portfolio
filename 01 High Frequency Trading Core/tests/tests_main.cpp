#include <catch2/catch_test_macros.hpp>
#include "../lib/hft_core_lib/hft_core.hpp"

using namespace std;

TEST_CASE("Price-Time Priority Logic", "[core][matching]") {
    SECTION("Price Priority: Higher Buy Price Fills First") {
        HFT::HFTCore engine;

        // Place two passive buy orders at different prices
        engine.LimitOrder({1, 100.0, 10, true, 1000}); // Order A: $100.0
        engine.LimitOrder({2, 101.0, 10, true, 1001}); // Order B: $101.0 (Better Price)

        // Place one aggressive sell order that can only fill 10 units
        engine.LimitOrder({3, 100.0, 10, false, 1002});

        // The better price ($101.0) must be filled first despite arriving later
        REQUIRE(engine.GetBuyBook().find(101.0) == engine.GetBuyBook().end());
        REQUIRE(engine.GetBuyBook().find(100.0) != engine.GetBuyBook().end());
        REQUIRE(engine.GetBuyBook().at(100.0).size() == 1);
    }

    SECTION("Time Priority: Older Order Fills First at Same Price") {
        HFT::HFTCore engine;

        // Place two buy orders at the same price with different timestamps
        engine.LimitOrder({1, 100.0, 10, true, 1000}); // Order A: Earlier
        engine.LimitOrder({2, 100.0, 10, true, 1001}); // Order B: Later

        // Place aggressive sell order for 10 units
        engine.LimitOrder({3, 100.0, 10, false, 1002});

        // The earlier order (Order A) must be filled
        auto& ordersAtPrice = engine.GetBuyBook().at(100.0);
        REQUIRE(ordersAtPrice.front().id == 2); // Only Order B remains
    }

    SECTION("Partial Fills and Remainder Persistence") {
        HFT::HFTCore engine;

        // Large Buy Order
        engine.LimitOrder({1, 100.0, 100, true, 1000});
        
        // Smaller Sell Order
        engine.LimitOrder({2, 100.0, 30, false, 1001});

        // Order 1 should have 70 units remaining in the book
        auto& ordersAtPrice = engine.GetBuyBook().at(100.0);
        REQUIRE(ordersAtPrice.front().quantity == 70);
    }

    SECTION("Timestamp Priority Works Even If Arrival Is Out Of Order") {
        HFT::HFTCore engine;

        // Insert newer order first, then older one at same price.
        engine.LimitOrder({2, 100.0, 10, true, 2000}); // Newer timestamp
        engine.LimitOrder({1, 100.0, 10, true, 1000}); // Older timestamp

        // Aggressive sell should match the older timestamp first.
        engine.LimitOrder({3, 100.0, 10, false, 3000});

        auto& ordersAtPrice = engine.GetBuyBook().at(100.0);
        REQUIRE(ordersAtPrice.front().id == 2);
    }
}