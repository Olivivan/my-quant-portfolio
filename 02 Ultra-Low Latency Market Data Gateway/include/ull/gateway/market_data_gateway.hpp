#pragma once

#include <atomic>

namespace ull::gateway {

class MarketDataGateway {
public:
    void start() noexcept;
    void stop() noexcept;
    [[nodiscard]] bool is_running() const noexcept;

private:
    std::atomic<bool> running_{false};
};

} // namespace ull::gateway
