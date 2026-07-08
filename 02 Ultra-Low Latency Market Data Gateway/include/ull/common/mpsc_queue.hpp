#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace ull::common {

template <typename T, std::size_t Capacity>
class MpscQueue {
    static_assert(Capacity >= 2, "Capacity must be at least 2");
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of two");

public:
    MpscQueue() noexcept {
        for (std::size_t i = 0; i < Capacity; ++i) {
            buffer_[i].sequence.store(i, std::memory_order_relaxed);
        }
    }

    bool try_enqueue(const T& value) noexcept {
        std::size_t pos = enqueue_pos_.load(std::memory_order_relaxed);

        for (;;) {
            auto& cell = buffer_[pos & mask_];
            const std::size_t seq = cell.sequence.load(std::memory_order_acquire);
            const std::intptr_t diff = static_cast<std::intptr_t>(seq) - static_cast<std::intptr_t>(pos);

            if (diff == 0) {
                if (enqueue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed, std::memory_order_relaxed)) {
                    cell.data = value;
                    cell.sequence.store(pos + 1, std::memory_order_release);
                    return true;
                }
            } else if (diff < 0) {
                return false;
            } else {
                pos = enqueue_pos_.load(std::memory_order_relaxed);
            }
        }
    }

    std::optional<T> try_dequeue() noexcept {
        std::size_t pos = dequeue_pos_.load(std::memory_order_relaxed);

        for (;;) {
            auto& cell = buffer_[pos & mask_];
            const std::size_t seq = cell.sequence.load(std::memory_order_acquire);
            const std::intptr_t diff = static_cast<std::intptr_t>(seq) - static_cast<std::intptr_t>(pos + 1);

            if (diff == 0) {
                dequeue_pos_.store(pos + 1, std::memory_order_relaxed);
                T value = cell.data;
                cell.sequence.store(pos + Capacity, std::memory_order_release);
                return value;
            }

            if (diff < 0) {
                return std::nullopt;
            }

            pos = dequeue_pos_.load(std::memory_order_relaxed);
        }
    }

private:
    struct Cell {
        std::atomic<std::size_t> sequence{0};
        T data{};
    };

    static constexpr std::size_t mask_ = Capacity - 1;

    std::array<Cell, Capacity> buffer_{};
    std::atomic<std::size_t> enqueue_pos_{0};
    std::atomic<std::size_t> dequeue_pos_{0};
};

} // namespace ull::common
