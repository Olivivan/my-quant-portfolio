#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

#include "Order.hpp"

class OrderPool {
public:
    using IndexType = std::size_t;
    using StorageType = std::vector<Order>;
    using FreeListType = std::vector<IndexType>;

    explicit OrderPool(IndexType capacity) noexcept
        : storage_(capacity), next_free_(capacity, std::numeric_limits<IndexType>::max()) {
        for (IndexType i = 0; i < capacity; ++i) {
            next_free_[i] = (i + 1 < capacity) ? i + 1 : std::numeric_limits<IndexType>::max();
        }
        free_head_ = 0;
    }

    [[nodiscard]] Order* allocate(Order::IdType id,
                                  bool is_buy,
                                  Order::PriceType price,
                                  Order::PriceType qty) noexcept {
        if (free_head_ == std::numeric_limits<IndexType>::max()) {
            return nullptr;
        }

        const IndexType index = free_head_;
        free_head_ = next_free_[index];

        Order& order = storage_[index];
        order.id = id;
        order.is_buy = is_buy;
        order.price = price;
        order.qty = qty;
        order.next = nullptr;
        order.prev = nullptr;
        return &order;
    }

    void release(Order* order) noexcept {
        if (order == nullptr) {
            return;
        }

        const IndexType index = static_cast<IndexType>(order - storage_.data());
        if (index >= storage_.size()) {
            return;
        }

        next_free_[index] = free_head_;
        free_head_ = index;
    }

    [[nodiscard]] bool has_capacity() const noexcept {
        return free_head_ != std::numeric_limits<IndexType>::max();
    }

private:
    StorageType storage_;
    FreeListType next_free_;
    IndexType free_head_{std::numeric_limits<IndexType>::max()};
};
