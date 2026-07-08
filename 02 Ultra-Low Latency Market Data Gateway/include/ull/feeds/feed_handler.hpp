#pragma once

#include <string_view>

namespace ull::feeds {

class FeedHandler {
public:
    [[nodiscard]] bool on_packet(std::string_view payload) noexcept;
};

} // namespace ull::feeds
