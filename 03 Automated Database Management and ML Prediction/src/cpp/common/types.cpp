#include "types.hpp"

namespace qp {

string side_to_string(Side s) {
    switch (s) {
        case Side::Buy: return "buy";
        case Side::Sell: return "sell";
        default: return "unknown";
    }
}

Side side_from_int(int8_t v) {
    if (v > 0) return Side::Buy;
    if (v < 0) return Side::Sell;
    return Side::Unknown;
}

} // namespace qp
