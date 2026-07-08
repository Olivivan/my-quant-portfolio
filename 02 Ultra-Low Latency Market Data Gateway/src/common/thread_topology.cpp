#include "ull/common/thread_topology.hpp"

#include <cstdlib>
#include <limits>
#include <utility>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <malloc.h>
#include <windows.h>
#elif defined(__linux__)
#include <pthread.h>
#include <sched.h>
#include <sys/mman.h>
#if __has_include(<linux/mempolicy.h>) && __has_include(<numaif.h>)
#define ULL_HAVE_NUMA_BIND 1
#include <linux/mempolicy.h>
#include <numaif.h>
#else
#define ULL_HAVE_NUMA_BIND 0
#endif
#else
#define ULL_HAVE_NUMA_BIND 0
#endif

namespace ull::common {

bool thread_pinning_supported() noexcept {
#if defined(_WIN32) || defined(__linux__)
    return true;
#else
    return false;
#endif
}

bool pin_current_thread_to_cpu(std::size_t cpu_index) noexcept {
    if (cpu_index >= (std::numeric_limits<unsigned int>::max)()) [[unlikely]] {
        return false;
    }

#if defined(_WIN32)
    if (cpu_index >= (sizeof(DWORD_PTR) * 8U)) [[unlikely]] {
        return false;
    }

    const DWORD_PTR affinity = (static_cast<DWORD_PTR>(1) << cpu_index);
    return SetThreadAffinityMask(GetCurrentThread(), affinity) != 0;
#elif defined(__linux__)
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(static_cast<int>(cpu_index), &set);
    return pthread_setaffinity_np(pthread_self(), sizeof(set), &set) == 0;
#else
    (void)cpu_index;
    return false;
#endif
}

NumaBuffer::NumaBuffer(NumaBuffer&& other) noexcept {
    *this = std::move(other);
}

NumaBuffer& NumaBuffer::operator=(NumaBuffer&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    release();
    data_ = std::exchange(other.data_, nullptr);
    size_ = std::exchange(other.size_, 0U);
    numa_bound_ = std::exchange(other.numa_bound_, false);
    backend_ = std::exchange(other.backend_, Backend::none);
    return *this;
}

NumaBuffer::~NumaBuffer() {
    release();
}

bool NumaBuffer::numa_allocation_supported() noexcept {
#if defined(_WIN32)
    ULONG highest_node = 0;
    return GetNumaHighestNodeNumber(&highest_node) != 0;
#elif defined(__linux__) && (ULL_HAVE_NUMA_BIND == 1)
    return true;
#else
    return false;
#endif
}

NumaBuffer NumaBuffer::allocate(std::size_t bytes, std::uint32_t numa_node) noexcept {
    constexpr std::size_t alignment = 64;
    if (bytes == 0) [[unlikely]] {
        return {};
    }

    const std::size_t aligned_bytes = ((bytes + alignment - 1) / alignment) * alignment;

#if defined(_WIN32)
    void* ptr = nullptr;
    bool bound = false;
    if (numa_allocation_supported()) {
        ptr = VirtualAllocExNuma(GetCurrentProcess(), nullptr, aligned_bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE, numa_node);
        bound = (ptr != nullptr);
    }

    if (ptr == nullptr) {
        ptr = _aligned_malloc(aligned_bytes, alignment);
    }

    if (ptr == nullptr) {
        return {};
    }

    return NumaBuffer(static_cast<std::byte*>(ptr), aligned_bytes, bound, bound ? Backend::virtual_alloc_numa : Backend::aligned_malloc);
#elif defined(__linux__)
    void* ptr = mmap(nullptr, aligned_bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        return {};
    }

    bool bound = false;
#if ULL_HAVE_NUMA_BIND == 1
    constexpr std::size_t bits_per_word = sizeof(unsigned long) * 8U;
    const std::uint32_t safe_node = static_cast<std::uint32_t>(numa_node % bits_per_word);
    unsigned long nodemask = (1UL << safe_node);
    if (mbind(ptr, aligned_bytes, MPOL_BIND, &nodemask, bits_per_word, 0) == 0) {
        bound = true;
    }
#else
    (void)numa_node;
#endif

    return NumaBuffer(static_cast<std::byte*>(ptr), aligned_bytes, bound, Backend::mmap);
#else
    (void)numa_node;
    void* ptr = std::aligned_alloc(alignment, aligned_bytes);
    if (ptr == nullptr) {
        return {};
    }

    return NumaBuffer(static_cast<std::byte*>(ptr), aligned_bytes, false, Backend::aligned_alloc);
#endif
}

bool NumaBuffer::valid() const noexcept {
    return data_ != nullptr;
}

bool NumaBuffer::numa_bound() const noexcept {
    return numa_bound_;
}

std::size_t NumaBuffer::size() const noexcept {
    return size_;
}

std::span<std::byte> NumaBuffer::bytes() noexcept {
    return {data_, size_};
}

std::span<const std::byte> NumaBuffer::bytes() const noexcept {
    return {data_, size_};
}

NumaBuffer::NumaBuffer(std::byte* data, std::size_t size, bool bound, Backend backend) noexcept
    : data_(data),
      size_(size),
      numa_bound_(bound),
      backend_(backend) {}

void NumaBuffer::release() noexcept {
    if (data_ == nullptr) {
        return;
    }

    switch (backend_) {
    case Backend::virtual_alloc_numa:
#if defined(_WIN32)
        VirtualFree(data_, 0, MEM_RELEASE);
#endif
        break;
    case Backend::aligned_malloc:
#if defined(_WIN32)
        _aligned_free(data_);
#endif
        break;
    case Backend::mmap:
#if defined(__linux__)
        munmap(data_, size_);
#endif
        break;
    case Backend::aligned_alloc:
        std::free(data_);
        break;
    case Backend::none:
        break;
    }

    data_ = nullptr;
    size_ = 0;
    numa_bound_ = false;
    backend_ = Backend::none;
}

} // namespace ull::common
