#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace ull::common {

[[nodiscard]] bool thread_pinning_supported() noexcept;
[[nodiscard]] bool pin_current_thread_to_cpu(std::size_t cpu_index) noexcept;

class NumaBuffer {
public:
    NumaBuffer() = default;

    NumaBuffer(const NumaBuffer&) = delete;
    NumaBuffer& operator=(const NumaBuffer&) = delete;

    NumaBuffer(NumaBuffer&& other) noexcept;
    NumaBuffer& operator=(NumaBuffer&& other) noexcept;
    ~NumaBuffer();

    [[nodiscard]] static bool numa_allocation_supported() noexcept;
    [[nodiscard]] static NumaBuffer allocate(std::size_t bytes, std::uint32_t numa_node = 0) noexcept;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] bool numa_bound() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] std::span<std::byte> bytes() noexcept;
    [[nodiscard]] std::span<const std::byte> bytes() const noexcept;

private:
    enum class Backend : std::uint8_t {
        none,
        virtual_alloc_numa,
        aligned_malloc,
        mmap,
        aligned_alloc,
    };

    NumaBuffer(std::byte* data, std::size_t size, bool bound, Backend backend) noexcept;
    void release() noexcept;

    std::byte* data_{nullptr};
    std::size_t size_{0};
    bool numa_bound_{false};
    Backend backend_{Backend::none};
};

} // namespace ull::common
