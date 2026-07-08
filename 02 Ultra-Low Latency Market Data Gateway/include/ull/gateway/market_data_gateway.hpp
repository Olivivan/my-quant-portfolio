#pragma once

#include "ull/common/fnv1a.hpp"
#include "ull/common/mpsc_queue.hpp"
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

enum class DeferredTaskKind : std::uint8_t {
    logging,
    persistence,
};

struct DeferredTask {
    DeferredTaskKind kind{DeferredTaskKind::logging};
    std::uint64_t payload{0};
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

    [[nodiscard]] bool enqueue_deferred_task(DeferredTask task) noexcept;
    [[nodiscard]] std::size_t process_deferred_tasks(std::size_t max_tasks) noexcept;
    [[nodiscard]] std::uint64_t deferred_processed_count() const noexcept;

private:
    std::atomic<bool> running_{false};
    std::atomic<std::uint64_t> routed_count_{0};
    ull::common::MpscQueue<DeferredTask, 1024> deferred_queue_{};
    std::atomic<std::uint64_t> deferred_processed_{0};
    ull::common::NumaBuffer numa_working_set_{};
};

} // namespace ull::gateway
