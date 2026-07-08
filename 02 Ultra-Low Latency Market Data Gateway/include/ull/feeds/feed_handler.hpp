#pragma once

#include "ull/feeds/data_access_view.hpp"
#include "ull/feeds/structural_scan.hpp"

#include <string_view>

namespace ull::feeds {

class FeedHandler {
public:
    [[nodiscard]] bool on_packet(std::string_view payload) noexcept;

private:
    StructuralScanner scanner_{};
    ParseContext parse_context_{};
};

} // namespace ull::feeds
