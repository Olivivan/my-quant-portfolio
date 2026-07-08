#pragma once

#include <array>
#include <cstddef>
#include <string_view>

namespace ull::feeds {

struct FieldSlice {
    std::size_t key_offset{0};
    std::size_t key_length{0};
    std::size_t value_offset{0};
    std::size_t value_length{0};
};

struct StructuralScanResult {
    static constexpr std::size_t max_fields = 32;

    bool valid{false};
    std::size_t field_count{0};
    std::array<FieldSlice, max_fields> fields{};
};

class StructuralScanner {
public:
    [[nodiscard]] StructuralScanResult scan(std::string_view payload) const noexcept;
};

} // namespace ull::feeds
