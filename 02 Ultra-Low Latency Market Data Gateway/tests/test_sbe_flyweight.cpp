#include "ull/sbe/market_data_flyweight.hpp"

#include <catch2/catch.hpp>

#include <array>
#include <cstddef>
#include <span>

TEST_CASE("SBE flyweight writes and reads directly over raw buffer", "[sbe][flyweight]") {
    std::array<std::byte, ull::sbe::MarketDataFlyweight::encoded_length> wire{};

    auto writer_opt = ull::sbe::MarketDataFlyweight::wrap(std::span<std::byte>(wire));
    REQUIRE(writer_opt.has_value());

    auto writer = *writer_opt;
    writer.initialize_header();
    writer.entry().instrument_id = 4242;
    writer.entry().price_e4 = 1234567;
    writer.entry().quantity = 250;
    writer.entry().side = ull::sbe::Side::buy;
    writer.entry().flags = 0x3;

    auto reader_opt = ull::sbe::MarketDataFlyweightConst::wrap(std::span<const std::byte>(wire));
    REQUIRE(reader_opt.has_value());

    const auto reader = *reader_opt;
    REQUIRE(reader.header().template_id == ull::sbe::MarketDataFlyweightConst::template_id_value);
    REQUIRE(reader.header().schema_id == ull::sbe::MarketDataFlyweightConst::schema_id_value);
    REQUIRE(reader.entry().instrument_id == 4242);
    REQUIRE(reader.entry().price_e4 == 1234567);
    REQUIRE(reader.entry().quantity == 250);
    REQUIRE(reader.entry().side == ull::sbe::Side::buy);
    REQUIRE(reader.entry().flags == 0x3);
}

TEST_CASE("SBE flyweight overlay points into original network buffer", "[sbe][flyweight]") {
    std::array<std::byte, ull::sbe::MarketDataFlyweight::encoded_length> wire{};

    auto writer_opt = ull::sbe::MarketDataFlyweight::wrap(std::span<std::byte>(wire));
    REQUIRE(writer_opt.has_value());

    auto writer = *writer_opt;
    writer.initialize_header();

    const auto* header_addr = reinterpret_cast<const std::byte*>(&writer.header());
    const auto* body_addr = reinterpret_cast<const std::byte*>(&writer.entry());

    REQUIRE(header_addr == wire.data());
    REQUIRE(body_addr == wire.data() + sizeof(ull::sbe::MessageHeader));
}

TEST_CASE("SBE flyweight rejects undersized buffers", "[sbe][flyweight]") {
    std::array<std::byte, ull::sbe::MarketDataFlyweight::encoded_length - 1> short_wire{};

    const auto writer_opt = ull::sbe::MarketDataFlyweight::wrap(std::span<std::byte>(short_wire));
    REQUIRE_FALSE(writer_opt.has_value());

    const auto reader_opt = ull::sbe::MarketDataFlyweightConst::wrap(std::span<const std::byte>(short_wire));
    REQUIRE_FALSE(reader_opt.has_value());
}
