#include "ull/feeds/data_access_view.hpp"
#include "ull/feeds/feed_handler.hpp"
#include "ull/feeds/structural_scan.hpp"

#include <catch2/catch.hpp>

namespace {

constexpr char SOH = '\x01';

} // namespace

TEST_CASE("Structural scanner extracts key/value boundaries", "[feeds][scan]") {
    const std::string payload = std::string{"sym=AAPL"} + SOH + "px=214.37" + SOH + "qty=120";

    ull::feeds::StructuralScanner scanner;
    const auto scan = scanner.scan(payload);

    REQUIRE(scan.valid);
    REQUIRE(scan.field_count == 3);
}

TEST_CASE("Data access resolves typed fields from scan metadata", "[feeds][access]") {
    const std::string payload = std::string{"sym=MSFT"} + SOH + "px=511.50" + SOH + "qty=90";

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

    REQUIRE(handler.on_packet(std::string{"sym=NVDA"} + SOH + "px=1421.10" + SOH + "qty=50"));
    REQUIRE_FALSE(handler.on_packet(std::string{"sym=NVDA"} + SOH + "px=bad" + SOH + "qty=50"));
    REQUIRE_FALSE(handler.on_packet(std::string{"sym=NVDA"} + SOH + "qty=50"));
    REQUIRE_FALSE(handler.on_packet(std::string{"sym="} + SOH + "px=1421.10" + SOH + "qty=50"));
    REQUIRE_FALSE(handler.on_packet(std::string{"sym=NVDA"} + SOH + "px=1421.10" + SOH + "qty=0"));
}

TEST_CASE("Structural scanner rejects trailing SOH delimiter", "[feeds][scan]") {
    const std::string payload = std::string{"sym=IBM"} + SOH + "px=184.5" + SOH;

    ull::feeds::StructuralScanner scanner;
    const auto scan = scanner.scan(payload);

    REQUIRE_FALSE(scan.valid);
}
