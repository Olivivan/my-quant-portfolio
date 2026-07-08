#include "ull/feeds/feed_handler.hpp"

namespace ull::feeds {

bool FeedHandler::on_packet(std::string_view payload) noexcept {
    return !payload.empty();
}

} // namespace ull::feeds
