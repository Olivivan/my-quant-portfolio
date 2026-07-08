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

} // namespace ull::gateway
