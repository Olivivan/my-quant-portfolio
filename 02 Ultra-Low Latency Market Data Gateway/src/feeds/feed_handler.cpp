#include "ull/feeds/feed_handler.hpp"

namespace ull::feeds {

bool FeedHandler::on_packet(std::string_view payload) noexcept {
    const auto scan = scanner_.scan(payload, parse_context_);
    if (!scan.valid) {
        return false;
    }

    const DataAccessView access(payload, scan);
    const auto symbol = access.get_string("sym");
    const auto price = access.get_double("px");
    const auto quantity = access.get_uint32("qty");

    return symbol.has_value() && !symbol->empty() && price.has_value() && quantity.has_value() && *quantity > 0U;
}

} // namespace ull::feeds
