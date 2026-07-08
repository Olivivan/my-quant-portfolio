#include "ull/feeds/structural_scan.hpp"

namespace ull::feeds {

StructuralScanResult StructuralScanner::scan(std::string_view payload) const noexcept {
    StructuralScanResult result{};
    if (payload.empty()) {
        return result;
    }

    std::size_t cursor = 0;
    while (cursor < payload.size()) {
        if (result.field_count >= StructuralScanResult::max_fields) {
            return result;
        }

        const auto field_end = payload.find(';', cursor);
        const auto segment_end = (field_end == std::string_view::npos) ? payload.size() : field_end;
        if (segment_end == cursor) {
            return result;
        }

        const auto separator = payload.find('=', cursor);
        if (separator == std::string_view::npos || separator >= segment_end || separator == cursor || separator + 1 >= segment_end) {
            return result;
        }

        result.fields[result.field_count++] = FieldSlice{
            cursor,
            separator - cursor,
            separator + 1,
            segment_end - (separator + 1)};

        if (field_end == std::string_view::npos) {
            break;
        }

        cursor = field_end + 1;
        if (cursor == payload.size()) {
            return result;
        }
    }

    result.valid = (result.field_count > 0);
    return result;
}

} // namespace ull::feeds
