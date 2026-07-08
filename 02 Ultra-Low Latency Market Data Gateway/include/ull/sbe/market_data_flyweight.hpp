#pragma once

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace ull::sbe {

enum class Side : std::uint8_t {
    buy = 1,
    sell = 2,
};

#pragma pack(push, 1)
struct MessageHeader {
    std::uint16_t block_length;
    std::uint16_t template_id;
    std::uint16_t schema_id;
    std::uint16_t version;
};

struct MarketDataEntry {
    std::uint32_t instrument_id;
    std::int64_t price_e4;
    std::uint32_t quantity;
    Side side;
    std::uint8_t flags;
};
#pragma pack(pop)

static_assert(sizeof(MessageHeader) == 8, "MessageHeader wire size must be 8 bytes");
static_assert(sizeof(MarketDataEntry) == 18, "MarketDataEntry wire size must be 18 bytes");

class MarketDataFlyweightConst {
public:
    static constexpr std::uint16_t template_id_value = 1001;
    static constexpr std::uint16_t schema_id_value = 1;
    static constexpr std::uint16_t version_value = 1;
    static constexpr std::size_t encoded_length = sizeof(MessageHeader) + sizeof(MarketDataEntry);

    static std::optional<MarketDataFlyweightConst> wrap(std::span<const std::byte> buffer) noexcept {
        if (buffer.size() < encoded_length) {
            return std::nullopt;
        }

        return MarketDataFlyweightConst(buffer);
    }

    [[nodiscard]] const MessageHeader& header() const noexcept {
        return *reinterpret_cast<const MessageHeader*>(buffer_.data());
    }

    [[nodiscard]] const MarketDataEntry& entry() const noexcept {
        return *reinterpret_cast<const MarketDataEntry*>(buffer_.data() + sizeof(MessageHeader));
    }

private:
    explicit MarketDataFlyweightConst(std::span<const std::byte> buffer) noexcept
        : buffer_(buffer.subspan(0, encoded_length)) {}

    std::span<const std::byte> buffer_{};
};

class MarketDataFlyweight {
public:
    static constexpr std::size_t encoded_length = MarketDataFlyweightConst::encoded_length;

    static std::optional<MarketDataFlyweight> wrap(std::span<std::byte> buffer) noexcept {
        if (buffer.size() < encoded_length) {
            return std::nullopt;
        }

        return MarketDataFlyweight(buffer);
    }

    void initialize_header() noexcept {
        auto& h = header();
        h.block_length = static_cast<std::uint16_t>(sizeof(MarketDataEntry));
        h.template_id = MarketDataFlyweightConst::template_id_value;
        h.schema_id = MarketDataFlyweightConst::schema_id_value;
        h.version = MarketDataFlyweightConst::version_value;
    }

    [[nodiscard]] MessageHeader& header() noexcept {
        return *reinterpret_cast<MessageHeader*>(buffer_.data());
    }

    [[nodiscard]] MarketDataEntry& entry() noexcept {
        return *reinterpret_cast<MarketDataEntry*>(buffer_.data() + sizeof(MessageHeader));
    }

    [[nodiscard]] const MessageHeader& header() const noexcept {
        return *reinterpret_cast<const MessageHeader*>(buffer_.data());
    }

    [[nodiscard]] const MarketDataEntry& entry() const noexcept {
        return *reinterpret_cast<const MarketDataEntry*>(buffer_.data() + sizeof(MessageHeader));
    }

private:
    explicit MarketDataFlyweight(std::span<std::byte> buffer) noexcept
        : buffer_(buffer.subspan(0, encoded_length)) {}

    std::span<std::byte> buffer_{};
};

} // namespace ull::sbe
