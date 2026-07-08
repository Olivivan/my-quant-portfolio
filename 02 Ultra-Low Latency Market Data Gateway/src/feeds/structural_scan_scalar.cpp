#include "structural_scan_context.hpp"

namespace ull::feeds::detail {

void scan_structural_scalar(std::string_view payload, ScanContext& context) noexcept {
    for (std::size_t i = 0; i < payload.size(); ++i) {
        const char ch = payload[i];
        if (ch == key_value_delimiter || ch == soh_delimiter) [[unlikely]] {
            if (!append_marker_or_fail(context, i, ch)) {
                return;
            }
        }
    }
}

} // namespace ull::feeds::detail
