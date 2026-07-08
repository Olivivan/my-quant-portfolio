#include <catch2/catch_test_macros.hpp>
#include "../lib/hft_core_lib/hft_core.hpp"

TEST_CASE("Price-Time Priority Logic", "[core][matching]") {
    HFT::HFTCore engine;

    SECTION("Price Priority: Higher Buy Price Fills First") {
        // Place two passive buy orders at different prices
        engine.LimitOrder({1, 100.0, 10, true, 1000}); // Order A: $100.0
        engine.LimitOrder({2, 101.0, 10, true, 1001}); // Order B: $101.0 (Better Price)

        // Place one aggressive sell order that can only fill 10 units
        engine.LimitOrder({3, 100.0, 10, false, 1002});

        // The better price ($101.0) must be filled first despite arriving later
        REQUIRE(engine.GetBuyBook().at(101.0).empty()); 
        REQUIRE(engine.GetBuyBook().at(100.0).size() == 1);
    }

    SECTION("Time Priority: Older Order Fills First at Same Price") {
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
        // Large Buy Order
        engine.LimitOrder({1, 100.0, 100, true, 1000});
        
        // Smaller Sell Order
        engine.LimitOrder({2, 100.0, 30, false, 1001});

        // Order 1 should have 70 units remaining in the book
        auto& ordersAtPrice = engine.GetBuyBook().at(100.0);
        REQUIRE(ordersAtPrice.front().quantity == 70);
    }
}