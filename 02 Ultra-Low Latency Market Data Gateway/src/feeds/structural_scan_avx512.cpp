#include "structural_scan_context.hpp"

#include <bit>
#include <cstdint>
#include <immintrin.h>

namespace ull::feeds::detail {

void scan_structural_avx512(std::string_view payload, ScanContext& context) noexcept {
    const auto* bytes = reinterpret_cast<const unsigned char*>(payload.data());
    const auto eq = _mm512_set1_epi8(static_cast<char>(key_value_delimiter));
    const auto soh = _mm512_set1_epi8(static_cast<char>(soh_delimiter));

    std::size_t offset = 0;
    const std::size_t simd_limit = payload.size() - (payload.size() % 64);

    while (offset < simd_limit) {
        const auto block = _mm512_loadu_si512(reinterpret_cast<const void*>(bytes + offset));
        const std::uint64_t eq_mask = _mm512_cmpeq_epi8_mask(block, eq);
        const std::uint64_t soh_mask = _mm512_cmpeq_epi8_mask(block, soh);

        std::uint64_t mask = eq_mask | soh_mask;
        while (mask != 0U) {
            const std::uint64_t bit = std::countr_zero(mask);
            const std::uint64_t bit_flag = (1ULL << bit);
            const char marker = ((eq_mask & bit_flag) != 0U) ? key_value_delimiter : soh_delimiter;

            if (!append_structural_marker(context, offset + static_cast<std::size_t>(bit), marker)) {
                context.failed = true;
                return;
            }

            mask &= ~bit_flag;
        }

        offset += 64;
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
