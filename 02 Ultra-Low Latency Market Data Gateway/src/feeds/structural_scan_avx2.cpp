#include "structural_scan_context.hpp"

#include <bit>
#include <cstdint>
#include <immintrin.h>

namespace ull::feeds::detail {

void scan_structural_avx2(std::string_view payload, ScanContext& context) noexcept {
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

            if (!append_structural_marker(context, offset + bit, marker)) {
                context.failed = true;
                return;
            }

            mask &= ~bit_flag;
        }

        offset += 32;
    }

    while (offset < payload.size()) {
        const char ch = payload[offset];
        if (ch == key_value_delimiter || ch == soh_delimiter) {
            if (!append_structural_marker(context, offset, ch)) {
                context.failed = true;
                return;
            }
        }

        ++offset;
    }
}

} // namespace ull::feeds::detail
