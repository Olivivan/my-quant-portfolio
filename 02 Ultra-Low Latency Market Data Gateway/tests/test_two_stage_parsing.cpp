#include "ull/feeds/data_access_view.hpp"
#include "ull/feeds/feed_handler.hpp"
#include "ull/feeds/structural_scan.hpp"

#include <catch2/catch.hpp>

TEST_CASE("Structural scanner extracts key/value boundaries", "[feeds][scan]") {
    const std::string_view payload = "sym=AAPL;px=214.37;qty=120";

    ull::feeds::StructuralScanner scanner;
    const auto scan = scanner.scan(payload);

    REQUIRE(scan.valid);
    REQUIRE(scan.field_count == 3);
}

TEST_CASE("Data access resolves typed fields from scan metadata", "[feeds][access]") {
    const std::string_view payload = "sym=MSFT;px=511.50;qty=90";

    ull::feeds::StructuralScanner scanner;
    const auto scan = scanner.scan(payload);
    REQUIRE(scan.valid);

    const ull::feeds::DataAccessView access(payload, scan);
    REQUIRE(access.valid());

    const auto symbol = access.get_string("sym");
    const auto price = access.get_double("px");
    const auto quantity = access.get_uint32("qty");

    REQUIRE(symbol.has_value());
    REQUIRE(*symbol == "MSFT");
    REQUIRE(price.has_value());
    REQUIRE(*price == Approx(511.50));
    REQUIRE(quantity.has_value());
    REQUIRE(*quantity == 90);
}

TEST_CASE("Feed handler validates required fields using two-stage parser", "[feeds][handler]") {
    ull::feeds::FeedHandler handler;

    REQUIRE(handler.on_packet("sym=NVDA;px=1421.10;qty=50"));
    REQUIRE_FALSE(handler.on_packet("sym=NVDA;px=bad;qty=50"));
    REQUIRE_FALSE(handler.on_packet("sym=NVDA;qty=50"));
    REQUIRE_FALSE(handler.on_packet("sym=;px=1421.10;qty=50"));
    REQUIRE_FALSE(handler.on_packet("sym=NVDA;px=1421.10;qty=0"));
}
