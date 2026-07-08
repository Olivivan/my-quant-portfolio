#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

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

struct StructuralScanResult {
    static constexpr std::size_t max_fields = 32;
    static constexpr std::size_t index_capacity = 64;

    bool valid{false};
    std::size_t field_count{0};
    std::array<FieldSlice, max_fields> fields{};
    std::array<std::uint8_t, index_capacity> index_slots{};
};

class StructuralScanner {
public:
    [[nodiscard]] StructuralScanResult scan(std::string_view payload) const noexcept;
};

} // namespace ull::feeds
