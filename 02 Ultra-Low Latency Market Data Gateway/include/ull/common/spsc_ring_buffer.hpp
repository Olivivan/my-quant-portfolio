#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <optional>

namespace ull::common {

template <typename T, std::size_t Capacity>
class SpscRingBuffer {
    static_assert(Capacity > 1, "Capacity must be greater than 1");

public:
    bool try_push(const T& value) noexcept {
        const auto head = head_.load(std::memory_order_relaxed);
        const auto next_head = increment(head);

        if (next_head == tail_.load(std::memory_order_acquire)) {
            return false;
        }

        buffer_[head] = value;
        head_.store(next_head, std::memory_order_release);
        return true;
    }

    std::optional<T> try_pop() noexcept {
        const auto tail = tail_.load(std::memory_order_relaxed);
        if (tail == head_.load(std::memory_order_acquire)) {
            return std::nullopt;
        }

        auto value = buffer_[tail];
        tail_.store(increment(tail), std::memory_order_release);
        return value;
    }

    [[nodiscard]] bool empty() const noexcept {
        return tail_.load(std::memory_order_acquire) == head_.load(std::memory_order_acquire);
    }

private:
    [[nodiscard]] static constexpr std::size_t increment(std::size_t idx) noexcept {
        return (idx + 1) % Capacity;
    }

    std::array<T, Capacity> buffer_{};
    std::atomic<std::size_t> head_{0};
    std::atomic<std::size_t> tail_{0};
};

} // namespace ull::common
