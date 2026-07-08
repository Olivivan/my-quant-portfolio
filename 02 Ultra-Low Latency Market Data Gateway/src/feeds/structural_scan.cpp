#include "ull/feeds/structural_scan.hpp"

#include "structural_scan_context.hpp"

#include <intrin.h>

namespace ull::feeds::detail {

namespace {

[[nodiscard]] bool build_structural_index(std::string_view payload, StructuralScanResult& result) noexcept {
    constexpr std::size_t mask = StructuralScanResult::index_capacity - 1;

    for (std::size_t field_index = 0; field_index < result.field_count; ++field_index) {
        const auto& field = result.fields[field_index];
        const auto key = payload.substr(field.key_offset, field.key_length);

        if (const auto known_tag = lookup_feed_tag(key); known_tag.has_value()) {
            const auto tag_index = static_cast<std::size_t>(*known_tag);
            if (result.known_tag_slots[tag_index] == 0U) {
                result.known_tag_slots[tag_index] = static_cast<std::uint8_t>(field_index + 1);
            }
        }

        std::size_t slot = static_cast<std::size_t>(structural_key_hash(key)) & mask;

        bool resolved = false;
        for (std::size_t probe = 0; probe < StructuralScanResult::index_capacity; ++probe) {
            auto& entry = result.index_slots[slot];
            if (entry == 0U) {
                entry = static_cast<std::uint8_t>(field_index + 1);
                resolved = true;
                break;
            }

            const std::size_t existing_index = static_cast<std::size_t>(entry - 1U);
            const auto& existing = result.fields[existing_index];
            const auto existing_key = payload.substr(existing.key_offset, existing.key_length);
            if (existing_key == key) {
                resolved = true;
                break;
            }

            slot = (slot + 1) & mask;
        }

        if (!resolved) {
            return false;
        }
    }

    return true;
}

[[nodiscard]] bool supports_avx2_runtime() noexcept {
    int cpuid[4]{};
    __cpuidex(cpuid, 1, 0);

    const bool osxsave = (cpuid[2] & (1 << 27)) != 0;
    const bool avx = (cpuid[2] & (1 << 28)) != 0;
    if (!osxsave || !avx) {
        return false;
    }

    const unsigned __int64 xcr0 = _xgetbv(0);
    if ((xcr0 & 0x6) != 0x6) {
        return false;
    }

    __cpuidex(cpuid, 7, 0);
    return (cpuid[1] & (1 << 5)) != 0;
}

[[nodiscard]] bool supports_avx512_runtime() noexcept {
    int cpuid[4]{};
    __cpuidex(cpuid, 1, 0);

    const bool osxsave = (cpuid[2] & (1 << 27)) != 0;
    if (!osxsave) {
        return false;
    }

    const unsigned __int64 xcr0 = _xgetbv(0);
    const unsigned __int64 required = 0xE6;
    if ((xcr0 & required) != required) {
        return false;
    }

    __cpuidex(cpuid, 7, 0);
    const bool avx512f = (cpuid[1] & (1 << 16)) != 0;
    const bool avx512bw = (cpuid[1] & (1 << 30)) != 0;
    return avx512f && avx512bw;
}

[[nodiscard]] ScanKernel resolve_scan_kernel() noexcept {
#if defined(ULL_HAS_AVX512_SCANNER)
    if (supports_avx512_runtime()) {
        return &scan_structural_avx512;
    }
#endif

#if defined(ULL_HAS_AVX2_SCANNER)
    if (supports_avx2_runtime()) {
        return &scan_structural_avx2;
    }
#endif

    return &scan_structural_scalar;
}

[[nodiscard]] ScanKernel get_scan_kernel() noexcept {
    static const ScanKernel kernel = resolve_scan_kernel();
    return kernel;
}

} // namespace

} // namespace ull::feeds::detail

namespace ull::feeds {

StructuralScanResult StructuralScanner::scan(std::string_view payload) const noexcept {
    ParseContext parse_context;
    return scan(payload, parse_context);
}

StructuralScanResult StructuralScanner::scan(std::string_view payload, ParseContext& parse_context) const noexcept {
    detail::ScanContext context{};
    if (payload.empty()) {
        return context.result;
    }

    detail::initialize_scan_context(context, parse_context);

    detail::get_scan_kernel()(payload, context);
    if (context.failed) {
        return context.result;
    }

    for (const auto& marker : parse_context.markers()) {
        if (!detail::process_marker(context, marker.position, marker.marker)) {
            context.failed = true;
            return context.result;
        }
    }

    if (!detail::finalize_scan(context, payload.size())) {
        return context.result;
    }

    if (!detail::build_structural_index(payload, context.result)) {
        return context.result;
    }

    context.result.valid = (context.result.field_count > 0);
    return context.result;
}

} // namespace ull::feeds
