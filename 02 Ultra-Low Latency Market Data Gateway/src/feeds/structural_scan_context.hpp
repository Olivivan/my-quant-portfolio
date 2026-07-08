#pragma once

#include "ull/feeds/structural_scan.hpp"

#include <cstddef>
#include <string_view>

namespace ull::feeds::detail {

constexpr char soh_delimiter = '\x01';
constexpr char key_value_delimiter = '=';

struct ScanContext {
    StructuralScanResult result{};
    ParseContext* parse_context{nullptr};
    std::size_t field_start{0};
    std::size_t value_separator{std::string_view::npos};
    bool failed{false};
};

using ScanKernel = void (*)(std::string_view payload, ScanContext& context) noexcept;

inline void initialize_scan_context(ScanContext& context, ParseContext& parse_context) noexcept {
    context.parse_context = &parse_context;
    context.parse_context->reset();
    context.field_start = 0;
    context.value_separator = std::string_view::npos;
    context.failed = false;
}

inline bool append_structural_marker(ScanContext& context, std::size_t position, char marker) noexcept {
    if (context.parse_context == nullptr) {
        return false;
    }

    auto& markers = context.parse_context->markers();
    if (markers.size() >= ParseContext::max_structural_markers) {
        return false;
    }

    markers.emplace_back(StructuralMarker{position, marker});
    return true;
}

inline bool append_field(ScanContext& context, std::size_t segment_end) noexcept {
    if (context.result.field_count >= StructuralScanResult::max_fields) {
        return false;
    }

    if (context.value_separator == std::string_view::npos) {
        return false;
    }

    if (context.value_separator == context.field_start || context.value_separator + 1 >= segment_end) {
        return false;
    }

    context.result.fields[context.result.field_count++] = FieldSlice{
        context.field_start,
        context.value_separator - context.field_start,
        context.value_separator + 1,
        segment_end - (context.value_separator + 1)};

    context.field_start = segment_end + 1;
    context.value_separator = std::string_view::npos;
    return true;
}

inline bool process_marker(ScanContext& context, std::size_t position, char marker) noexcept {
    if (marker == key_value_delimiter) {
        if (context.value_separator != std::string_view::npos || position == context.field_start) {
            return false;
        }

        context.value_separator = position;
        return true;
    }

    if (position == context.field_start) {
        return false;
    }

    return append_field(context, position);
}

inline bool finalize_scan(ScanContext& context, std::size_t payload_size) noexcept {
    if (context.field_start == payload_size) {
        return false;
    }

    return append_field(context, payload_size);
}

void scan_structural_scalar(std::string_view payload, ScanContext& context) noexcept;

#if defined(ULL_HAS_AVX2_SCANNER)
void scan_structural_avx2(std::string_view payload, ScanContext& context) noexcept;
#endif

#if defined(ULL_HAS_AVX512_SCANNER)
void scan_structural_avx512(std::string_view payload, ScanContext& context) noexcept;
#endif

} // namespace ull::feeds::detail
