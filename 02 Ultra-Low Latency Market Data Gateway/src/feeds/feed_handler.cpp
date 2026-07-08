#include "ull/feeds/feed_handler.hpp"

namespace ull::feeds {

bool FeedHandler::on_packet(std::string_view payload) noexcept {
    const auto scan = scanner_.scan(payload, parse_context_);
    if (!scan.valid) [[unlikely]] {
        return false;
    }

    const DataAccessView access(payload, scan);
    const auto symbol = access.get_string(FeedTag::sym);
    const auto price = access.get_double(FeedTag::px);
    const auto quantity = access.get_uint32(FeedTag::qty);

    if (symbol.has_value() && !symbol->empty() && price.has_value() && quantity.has_value() && *quantity > 0U) [[likely]] {
        return true;
    }

    return false;
}

} // namespace ull::feeds
