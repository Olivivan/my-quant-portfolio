#include "ull/feeds/structural_scan.hpp"

#include "structural_scan_context.hpp"

#include <intrin.h>

namespace ull::feeds::detail {

namespace {

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
    detail::ScanContext context{};
    if (payload.empty()) {
        return context.result;
    }

    detail::get_scan_kernel()(payload, context);
    if (context.failed) {
        return context.result;
    }

    if (!detail::finalize_scan(context, payload.size())) {
        return context.result;
    }

    context.result.valid = (context.result.field_count > 0);
    return context.result;
}

} // namespace ull::feeds
