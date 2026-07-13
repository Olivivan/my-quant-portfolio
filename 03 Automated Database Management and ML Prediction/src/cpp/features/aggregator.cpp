#include "aggregator.hpp"
#include <algorithm>
#include <map>

using namespace std;

namespace qp::features {

BarAggregator::BarAggregator(chrono::seconds interval) : interval_(interval) {}

vector<Bar> BarAggregator::aggregate(const vector<CleanedTick>& ticks) const {
    if (ticks.empty()) return {};

    struct Bucket {
        double open = 0.0;
        double high = 0.0;
        double low = 0.0;
        double close = 0.0;
        double volume = 0.0;
        double vwap_num = 0.0;
        int32_t trades = 0;
    };

    map<Timestamp, Bucket> buckets;
    string symbol = ticks.front().symbol;

    for (const auto& t : ticks) {
        if (t.outlier) continue;
        auto ts = t.time;
        auto epoch = chrono::duration_cast<chrono::seconds>(ts.time_since_epoch());
        epoch = (epoch / interval_.count()) * interval_.count();
        Timestamp bucket_ts{chrono::seconds(epoch)};

        auto& b = buckets[bucket_ts];
        if (b.trades == 0) {
            b.open = b.high = b.low = b.close = t.price;
        } else {
            b.high = max(b.high, t.price);
            b.low = min(b.low, t.price);
            b.close = t.price;
        }
        b.volume += t.size;
        b.vwap_num += t.price * t.size;
        ++b.trades;
    }

    vector<Bar> bars;
    bars.reserve(buckets.size());
    for (auto& [ts, b] : buckets) {
        bars.push_back(Bar{
            .time = ts,
            .symbol = symbol,
            .open = b.open,
            .high = b.high,
            .low = b.low,
            .close = b.close,
            .volume = b.volume,
            .vwap = b.volume > 0.0 ? b.vwap_num / b.volume : b.close,
            .trades = b.trades
        });
    }
    return bars;
}

} // namespace qp::features
