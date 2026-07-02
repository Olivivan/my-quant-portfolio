#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "MemoryPool.hpp"

struct PriceLevel {
    using PriceType = std::uint32_t;
    using VolumeType = std::uint64_t;

    explicit PriceLevel(PriceType level_price) noexcept
        : price(level_price) {}

    PriceLevel() = default;

    [[nodiscard]] bool empty() const noexcept {
        return head == nullptr;
    }

    void append(Order* order) noexcept {
        if (order == nullptr) {
            return;
        }
        if (tail == nullptr) {
            head = order;
            tail = order;
            order->prev = nullptr;
            order->next = nullptr;
            return;
        }

        tail->next = order;
        order->prev = tail;
        order->next = nullptr;
        tail = order;
    }

    void erase(Order* order) noexcept {
        if (order == nullptr) {
            return;
        }
        if (order->prev != nullptr) {
            order->prev->next = order->next;
        } else {
            head = order->next;
        }

        if (order->next != nullptr) {
            order->next->prev = order->prev;
        } else {
            tail = order->prev;
        }

        order->prev = nullptr;
        order->next = nullptr;
    }

    PriceType price{0};
    VolumeType total_volume{0};
    Order* head{nullptr};
    Order* tail{nullptr};
};

class OrderBook {
public:
    using PoolCapacity = std::size_t;
    using IdType = std::uint64_t;
    using PriceType = std::uint32_t;
    using BidMap = std::map<PriceType, PriceLevel, std::greater<PriceType>>;
    using AskMap = std::map<PriceType, PriceLevel, std::less<PriceType>>;
    using ActiveOrderMap = std::unordered_map<IdType, Order*>;

    explicit OrderBook(PoolCapacity pool_capacity = 200'000, bool log_executions = true) noexcept;

    void submit_order(IdType id, bool is_buy, PriceType price, PriceType qty);
    void cancel_order(IdType id);

    [[nodiscard]] PoolCapacity active_order_count() const noexcept;
    [[nodiscard]] std::uint64_t total_executions() const noexcept;

private:
    void insert_resting_order(Order* order);
    void remove_resting_order(Order* order);

    OrderPool pool_;
    BidMap bids_;
    AskMap asks_;
    ActiveOrderMap active_orders_;
    std::uint64_t total_executions_{0};
    bool log_executions_{true};
};

struct OrderCommand {
    using IdType = std::uint64_t;
    using PriceType = std::uint32_t;

    IdType id{0};
    bool is_buy{false};
    PriceType price{0};
    PriceType qty{0};
};

class PriceLevelWorker {
public:
    using PoolCapacity = std::size_t;
    using IdType = std::uint64_t;
    using PriceType = std::uint32_t;

    explicit PriceLevelWorker(PoolCapacity pool_capacity,
                              PriceType partition_id,
                              PoolCapacity worker_count,
                              bool log_executions = false) noexcept;

    void set_peers(std::vector<PriceLevelWorker*> peers) noexcept;

    bool submit_order(IdType id, bool is_buy, PriceType price, PriceType qty);
    void cancel_order(IdType id);

    [[nodiscard]] PoolCapacity active_order_count() const noexcept;
    [[nodiscard]] std::uint64_t total_executions() const noexcept;

private:
    bool can_match(bool is_buy, PriceType incoming_price, PriceType level_price) const noexcept;
    void insert_resting_order(Order* order);
    void remove_resting_order(Order* order);

    PriceType partition_id_{0};
    PoolCapacity worker_count_{1};
    bool log_executions_{false};
    mutable std::mutex mutex_;
    OrderPool pool_;
    std::map<PriceType, PriceLevel, std::greater<PriceType>> bids_;
    std::map<PriceType, PriceLevel, std::less<PriceType>> asks_;
    std::unordered_map<IdType, Order*> active_orders_;
    std::uint64_t total_executions_{0};
    std::vector<PriceLevelWorker*> peers_;
};

class MultiThreadedOrderBook {
public:
    using PoolCapacity = std::size_t;

    explicit MultiThreadedOrderBook(PoolCapacity pool_capacity = 200'000,
                                    PoolCapacity worker_count = 4,
                                    PoolCapacity batch_size = 512) noexcept;

    void submit_batch(const std::vector<OrderCommand>& commands);

    [[nodiscard]] PoolCapacity active_order_count() const noexcept;
    [[nodiscard]] std::uint64_t total_executions() const noexcept;

private:
    PoolCapacity worker_count_{1};
    PoolCapacity batch_size_{512};
    std::vector<std::unique_ptr<PriceLevelWorker>> workers_;
};
