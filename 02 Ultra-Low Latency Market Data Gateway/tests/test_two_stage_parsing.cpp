#include "ull/feeds/data_access_view.hpp"
#include "ull/feeds/feed_handler.hpp"
#include "ull/feeds/structural_scan.hpp"
#include "ull/feeds/tag_lookup.hpp"

#include <catch2/catch.hpp>
#include <string>

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

TEST_CASE("Structural scanner handles payload across 64-byte blocks", "[feeds][scan]") {
    std::string payload;
    payload.reserve(256);
    payload += "sym=LONGSYMBOL1234";
    payload += SOH;
    payload += "px=12345.6789";
    payload += SOH;
    payload += "qty=999999";
    payload += SOH;
    payload += "venue=XNYS";
    payload += SOH;
    payload += "seq=184467";

    ull::feeds::StructuralScanner scanner;
    const auto scan = scanner.scan(payload);

    REQUIRE(scan.valid);
    REQUIRE(scan.field_count == 5);
}

TEST_CASE("Structural index supports O(1) lookup semantics", "[feeds][index]") {
    const std::string payload = std::string{"sym=AMD"} + SOH + "px=166.42" + SOH + "qty=700" + SOH + "venue=XNAS";

    ull::feeds::StructuralScanner scanner;
    const auto scan = scanner.scan(payload);
    REQUIRE(scan.valid);

    const ull::feeds::DataAccessView access(payload, scan);
    REQUIRE(access.get_string("sym") == std::optional<std::string_view>{"AMD"});
    const auto price = access.get_double("px");
    REQUIRE(price.has_value());
    REQUIRE(*price == Approx(166.42));
    REQUIRE(access.get_uint32("qty") == std::optional<std::uint32_t>{700});
    REQUIRE(access.get_string("missing") == std::nullopt);
}

TEST_CASE("Structural index preserves first occurrence for duplicate keys", "[feeds][index]") {
    const std::string payload = std::string{"sym=FIRST"} + SOH + "sym=SECOND" + SOH + "px=1.5" + SOH + "qty=10";

    ull::feeds::StructuralScanner scanner;
    const auto scan = scanner.scan(payload);
    REQUIRE(scan.valid);

    const ull::feeds::DataAccessView access(payload, scan);
    const auto symbol = access.get_string("sym");
    REQUIRE(symbol.has_value());
    REQUIRE(*symbol == "FIRST");
}

TEST_CASE("Consteval feed tag table resolves known tags", "[feeds][tags]") {
    constexpr auto sym = ull::feeds::lookup_feed_tag("sym");
    constexpr auto px = ull::feeds::lookup_feed_tag("px");
    constexpr auto unknown = ull::feeds::lookup_feed_tag("unknown_field");

    static_assert(sym.has_value());
    static_assert(px.has_value());
    static_assert(!unknown.has_value());
    static_assert(ull::feeds::feed_tag_name(ull::feeds::FeedTag::qty) == "qty");

    REQUIRE(sym.has_value());
    REQUIRE(*sym == ull::feeds::FeedTag::sym);
    REQUIRE(px.has_value());
    REQUIRE(*px == ull::feeds::FeedTag::px);
    REQUIRE_FALSE(unknown.has_value());
}

TEST_CASE("Data access resolves known tags via direct slot arrays", "[feeds][tags]") {
    const std::string payload = std::string{"sym=INTC"} + SOH + "px=52.75" + SOH + "qty=1200" + SOH + "venue=XNAS";

    ull::feeds::StructuralScanner scanner;
    const auto scan = scanner.scan(payload);
    REQUIRE(scan.valid);

    const ull::feeds::DataAccessView access(payload, scan);
    REQUIRE(access.get_string(ull::feeds::FeedTag::sym) == std::optional<std::string_view>{"INTC"});
    REQUIRE(access.get_string(ull::feeds::FeedTag::venue) == std::optional<std::string_view>{"XNAS"});

    const auto price = access.get_double(ull::feeds::FeedTag::px);
    REQUIRE(price.has_value());
    REQUIRE(*price == Approx(52.75));
    REQUIRE(access.get_uint32(ull::feeds::FeedTag::qty) == std::optional<std::uint32_t>{1200});
}

TEST_CASE("ParseContext can be reused across packets", "[feeds][pmr]") {
    ull::feeds::StructuralScanner scanner;
    ull::feeds::ParseContext parse_context;

    const std::string first_payload = std::string{"sym=TSLA"} + SOH + "px=299.10" + SOH + "qty=42";
    const std::string second_payload = std::string{"sym=ORCL"} + SOH + "px=140.25" + SOH + "qty=18";

    const auto first_scan = scanner.scan(first_payload, parse_context);
    REQUIRE(first_scan.valid);

    const auto second_scan = scanner.scan(second_payload, parse_context);
    REQUIRE(second_scan.valid);

    const ull::feeds::DataAccessView first_access(first_payload, first_scan);
    const ull::feeds::DataAccessView second_access(second_payload, second_scan);
    REQUIRE(first_access.get_string("sym") == std::optional<std::string_view>{"TSLA"});
    REQUIRE(second_access.get_string("sym") == std::optional<std::string_view>{"ORCL"});
}

TEST_CASE("ParseContext marker capacity rejects malformed oversized structural stream", "[feeds][pmr]") {
    ull::feeds::StructuralScanner scanner;
    ull::feeds::ParseContext parse_context;

    std::string payload;
    payload.reserve(ull::feeds::ParseContext::max_structural_markers + 8);
    for (std::size_t i = 0; i < ull::feeds::ParseContext::max_structural_markers + 1; ++i) {
        payload.push_back(SOH);
    }

    const auto scan = scanner.scan(payload, parse_context);
    REQUIRE_FALSE(scan.valid);
}
