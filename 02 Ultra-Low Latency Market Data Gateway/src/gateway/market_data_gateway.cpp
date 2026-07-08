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

bool MarketDataGateway::pin_execution_thread(std::size_t cpu_index) noexcept {
    return ull::common::pin_current_thread_to_cpu(cpu_index);
}

bool MarketDataGateway::allocate_numa_working_set(std::size_t bytes, std::uint32_t numa_node) noexcept {
    numa_working_set_ = ull::common::NumaBuffer::allocate(bytes, numa_node);
    return numa_working_set_.valid();
}

bool MarketDataGateway::numa_working_set_ready() const noexcept {
    return numa_working_set_.valid();
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

bool MarketDataGateway::enqueue_deferred_task(DeferredTask task) noexcept {
    return deferred_queue_.try_enqueue(task);
}

std::size_t MarketDataGateway::process_deferred_tasks(std::size_t max_tasks) noexcept {
    if (max_tasks == 0) {
        return 0;
    }

    std::size_t processed = 0;
    while (processed < max_tasks) {
        const auto task = deferred_queue_.try_dequeue();
        if (!task.has_value()) {
            break;
        }

        ++processed;
    }

    deferred_processed_.fetch_add(processed, std::memory_order_relaxed);
    return processed;
}

std::uint64_t MarketDataGateway::deferred_processed_count() const noexcept {
    return deferred_processed_.load(std::memory_order_relaxed);
}

} // namespace ull::gateway
