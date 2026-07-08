#pragma once

#include "ull/common/fnv1a.hpp"
#include "ull/common/thread_topology.hpp"

#include <atomic>
#include <cstdint>
#include <string_view>

namespace ull::gateway {

enum class MessageType : std::uint8_t {
    quote,
    trade,
    book_update,
    heartbeat,
    unknown,
};

class MarketDataGateway {
public:
    static constexpr std::uint64_t quote_hash = ull::common::fnv1a_64_literal("QUOTE", 5);
    static constexpr std::uint64_t trade_hash = ull::common::fnv1a_64_literal("TRADE", 5);
    static constexpr std::uint64_t book_update_hash = ull::common::fnv1a_64_literal("BOOK_UPDATE", 11);
    static constexpr std::uint64_t heartbeat_hash = ull::common::fnv1a_64_literal("HEARTBEAT", 9);

    void start() noexcept;
    void stop() noexcept;
    [[nodiscard]] bool is_running() const noexcept;

    [[nodiscard]] bool pin_execution_thread(std::size_t cpu_index) noexcept;
    [[nodiscard]] bool allocate_numa_working_set(std::size_t bytes, std::uint32_t numa_node) noexcept;
    [[nodiscard]] bool numa_working_set_ready() const noexcept;

    [[nodiscard]] MessageType route_message_type(std::string_view message_type) noexcept;
    [[nodiscard]] std::uint64_t routed_message_count() const noexcept;

private:
    std::atomic<bool> running_{false};
    std::atomic<std::uint64_t> routed_count_{0};
    ull::common::NumaBuffer numa_working_set_{};
};

} // namespace ull::gateway
