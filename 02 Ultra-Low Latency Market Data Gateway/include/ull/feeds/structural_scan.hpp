#pragma once

#include "ull/feeds/tag_lookup.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory_resource>
#include <string_view>
#include <vector>

namespace ull::feeds {

[[nodiscard]] inline std::uint64_t structural_key_hash(std::string_view key) noexcept {
    std::uint64_t hash = 14695981039346656037ULL;
    for (const unsigned char ch : key) {
        hash ^= static_cast<std::uint64_t>(ch);
        hash *= 1099511628211ULL;
    }

    return hash;
}

struct FieldSlice {
    std::size_t key_offset{0};
    std::size_t key_length{0};
    std::size_t value_offset{0};
    std::size_t value_length{0};
};

struct StructuralMarker {
    std::size_t position{0};
    char marker{0};
};

class ParseContext {
public:
    static constexpr std::size_t max_structural_markers = 128;
    static constexpr std::size_t arena_size_bytes = 8192;

    ParseContext()
        : resource_(arena_.data(), arena_.size(), std::pmr::null_memory_resource()),
          markers_(&resource_) {
        markers_.reserve(max_structural_markers);
    }

    void reset() noexcept {
        markers_.clear();
    }

    [[nodiscard]] std::pmr::vector<StructuralMarker>& markers() noexcept {
        return markers_;
    }

    [[nodiscard]] const std::pmr::vector<StructuralMarker>& markers() const noexcept {
        return markers_;
    }

private:
    std::array<std::byte, arena_size_bytes> arena_{};
    std::pmr::monotonic_buffer_resource resource_;
    std::pmr::vector<StructuralMarker> markers_;
};

struct StructuralScanResult {
    static constexpr std::size_t max_fields = 32;
    static constexpr std::size_t index_capacity = 64;

    bool valid{false};
    std::size_t field_count{0};
    std::array<FieldSlice, max_fields> fields{};
    std::array<std::uint8_t, index_capacity> index_slots{};
    std::array<std::uint8_t, feed_tag_count> known_tag_slots{};
};

class StructuralScanner {
public:
    [[nodiscard]] StructuralScanResult scan(std::string_view payload) const noexcept;
    [[nodiscard]] StructuralScanResult scan(std::string_view payload, ParseContext& parse_context) const noexcept;
};

} // namespace ull::feeds
