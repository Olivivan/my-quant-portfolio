#include "imputer.hpp"
#include "common/logger.hpp"
#include <algorithm>

using namespace std;

namespace qp::etl {

vector<Imputer::ImputedTick> Imputer::resample_and_impute(
    const vector<CleanedTick>& ticks,
    chrono::milliseconds interval) const {

    if (ticks.empty()) return {};

    auto start = ticks.front().time;
    auto end = ticks.back().time;

    // Round down start to nearest interval
    auto start_ms = chrono::duration_cast<chrono::milliseconds>(start.time_since_epoch());
    start_ms = (start_ms / interval) * interval;
    start = chrono::system_clock::time_point(start_ms);

    vector<ImputedTick> result;
    size_t estimate = static_cast<size_t>(
        chrono::duration_cast<chrono::milliseconds>(end - start).count() / interval.count()) + 1;
    result.reserve(estimate);

    size_t idx = 0;
    double last_price = ticks.front().price;
    double last_size = ticks.front().size;

    for (auto t = start; t <= end; t += interval) {
        // Advance idx to last tick <= t
        while (idx < ticks.size() && ticks[idx].time <= t) {
            if (!ticks[idx].outlier) {
                last_price = ticks[idx].price;
                last_size = ticks[idx].size;
            }
            ++idx;
        }

        bool generated = true;
        if (idx < ticks.size() && idx > 0) {
            // Linear interpolation if current interval falls between two valid ticks
            const auto& prev = ticks[idx - 1];
            const auto& next = ticks[idx];
            if (!prev.outlier && !next.outlier && next.time > prev.time) {
                double alpha = static_cast<double>(
                    chrono::duration_cast<chrono::milliseconds>(t - prev.time).count()) /
                    chrono::duration_cast<chrono::milliseconds>(next.time - prev.time).count();
                last_price = prev.price + alpha * (next.price - prev.price);
                last_size = prev.size + alpha * (next.size - prev.size);
                generated = false;
            }
        }

        result.push_back(ImputedTick{t, last_price, last_size, generated});
    }

    size_t generated_count = count_if(result.begin(), result.end(),
        [](const auto& x) { return x.generated; });
    QP_INFO("Imputer generated {} / {} resampled ticks", generated_count, result.size());

    return result;
}

} // namespace qp::etl
