#include "ull/feeds/structural_scan.hpp"

#include <bit>
#include <cstdint>

#if defined(__AVX2__)
#include <immintrin.h>
#endif

namespace ull::feeds {

StructuralScanResult StructuralScanner::scan(std::string_view payload) const noexcept {
    StructuralScanResult result{};
    if (payload.empty()) {
        return result;
    }

    constexpr char soh_delimiter = '\x01';
    constexpr char key_value_delimiter = '=';

    std::size_t field_start = 0;
    std::size_t value_separator = std::string_view::npos;

    const auto append_field = [&](std::size_t segment_end) noexcept -> bool {
        if (result.field_count >= StructuralScanResult::max_fields) {
            return false;
        }

        if (value_separator == std::string_view::npos) {
            return false;
        }

        if (value_separator == field_start || value_separator + 1 >= segment_end) {
            return false;
        }

        result.fields[result.field_count++] = FieldSlice{
            field_start,
            value_separator - field_start,
            value_separator + 1,
            segment_end - (value_separator + 1)};

        field_start = segment_end + 1;
        value_separator = std::string_view::npos;
        return true;
    };

    const auto on_structural_char = [&](std::size_t position, char ch) noexcept -> bool {
        if (ch == key_value_delimiter) {
            if (value_separator != std::string_view::npos || position == field_start) {
                return false;
            }

            value_separator = position;
            return true;
        }

        if (position == field_start) {
            return false;
        }

        return append_field(position);
    };

#if defined(__AVX2__)
    const auto* bytes = reinterpret_cast<const unsigned char*>(payload.data());
    const auto eq = _mm256_set1_epi8(static_cast<char>(key_value_delimiter));
    const auto soh = _mm256_set1_epi8(static_cast<char>(soh_delimiter));

    std::size_t offset = 0;
    const std::size_t simd_limit = payload.size() - (payload.size() % 32);

    while (offset < simd_limit) {
        const auto block = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(bytes + offset));
        const auto eq_mask = static_cast<std::uint32_t>(_mm256_movemask_epi8(_mm256_cmpeq_epi8(block, eq)));
        const auto soh_mask = static_cast<std::uint32_t>(_mm256_movemask_epi8(_mm256_cmpeq_epi8(block, soh)));

        std::uint32_t mask = eq_mask | soh_mask;
        while (mask != 0U) {
            const std::uint32_t bit = std::countr_zero(mask);
            const std::uint32_t bit_flag = (1U << bit);
            const char marker = ((eq_mask & bit_flag) != 0U) ? key_value_delimiter : soh_delimiter;

            if (!on_structural_char(offset + bit, marker)) {
                return result;
            }

            mask &= ~bit_flag;
        }

        offset += 32;
    }

    while (offset < payload.size()) {
        const char ch = payload[offset];
        if (ch == key_value_delimiter || ch == soh_delimiter) {
            if (!on_structural_char(offset, ch)) {
                return result;
            }
        }

        ++offset;
    }
#else
    for (std::size_t i = 0; i < payload.size(); ++i) {
        const char ch = payload[i];
        if (ch == key_value_delimiter || ch == soh_delimiter) {
            if (!on_structural_char(i, ch)) {
                return result;
            }
        }
    }
#endif

    if (field_start == payload.size()) {
        return result;
    }

    if (!append_field(payload.size())) {
        return result;
    }

    result.valid = (result.field_count > 0);
    return result;
}

} // namespace ull::feeds
