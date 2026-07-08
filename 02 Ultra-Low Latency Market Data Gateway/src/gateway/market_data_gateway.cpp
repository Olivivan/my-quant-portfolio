#include "ull/gateway/market_data_gateway.hpp"

namespace ull::gateway {

void MarketDataGateway::start() noexcept {
    running_.store(true, std::memory_order_release);
}

void MarketDataGateway::stop() noexcept {
    running_.store(false, std::memory_order_release);
}

bool MarketDataGateway::is_running() const noexcept {
    return running_.load(std::memory_order_acquire);
}

MessageType MarketDataGateway::route_message_type(std::string_view message_type) noexcept {
    const std::uint64_t hash = ull::common::fnv1a_64(message_type);

    MessageType routed_type = MessageType::unknown;
    switch (hash) {
    case quote_hash:
        if (message_type == "QUOTE") [[likely]] {
            routed_type = MessageType::quote;
        } else [[unlikely]] {
            routed_type = MessageType::unknown;
        }
        break;
    case trade_hash:
        if (message_type == "TRADE") [[likely]] {
            routed_type = MessageType::trade;
        } else [[unlikely]] {
            routed_type = MessageType::unknown;
        }
        break;
    case book_update_hash:
        if (message_type == "BOOK_UPDATE") [[likely]] {
            routed_type = MessageType::book_update;
        } else [[unlikely]] {
            routed_type = MessageType::unknown;
        }
        break;
    case heartbeat_hash:
        if (message_type == "HEARTBEAT") [[likely]] {
            routed_type = MessageType::heartbeat;
        } else [[unlikely]] {
            routed_type = MessageType::unknown;
        }
        break;
    default:
        [[unlikely]]
        routed_type = MessageType::unknown;
        break;
    }

    routed_count_.fetch_add(1, std::memory_order_relaxed);
    return routed_type;
}

std::uint64_t MarketDataGateway::routed_message_count() const noexcept {
    return routed_count_.load(std::memory_order_relaxed);
}

} // namespace ull::gateway
