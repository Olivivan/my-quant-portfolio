#include "cleaner.hpp"
#include "common/logger.hpp"
#include <algorithm>
#include <cmath>
#include <execution>

using namespace std;

namespace qp::etl {

Cleaner::Cleaner(CleaningConfig cfg) : cfg_(cfg) {}

vector<CleanedTick> Cleaner::clean(vector<Tick>&& ticks) const {
    vector<CleanedTick> cleaned;
    cleaned.reserve(ticks.size());
    for (auto& t : ticks) {
        cleaned.push_back(CleanedTick{
            t.time,
            move(t.symbol),
            t.price,
            t.size,
            t.side,
            move(t.source),
            false,
            false
        });
    }

    if (cleaned.empty()) return cleaned;

    flag_jump_outliers(cleaned);
    flag_mad_outliers(cleaned);
    winsorize_prices(cleaned);

    size_t outliers = count_if(cleaned.begin(), cleaned.end(),
        [](const CleanedTick& t) { return t.outlier; });
    QP_INFO("Cleaner flagged {} / {} ticks as outliers", outliers, cleaned.size());

    return cleaned;
}

void Cleaner::winsorize_prices(vector<CleanedTick>& ticks) const {
    vector<double> prices;
    prices.reserve(ticks.size());
    for (const auto& t : ticks) prices.push_back(t.price);

    sort(prices.begin(), prices.end());
    size_t n = prices.size();
    double lower = prices[static_cast<size_t>(cfg_.winsorize_quantile * n)];
    double upper = prices[static_cast<size_t>((1.0 - cfg_.winsorize_quantile) * n)];

    for_each(execution::par_unseq, ticks.begin(), ticks.end(),
        [lower, upper](CleanedTick& t) {
            if (t.price < lower) { t.price = lower; t.outlier = true; }
            if (t.price > upper) { t.price = upper; t.outlier = true; }
        });
}

void Cleaner::flag_mad_outliers(vector<CleanedTick>& ticks) const {
    vector<double> prices(ticks.size());
    for (size_t i = 0; i < ticks.size(); ++i) prices[i] = ticks[i].price;

    nth_element(prices.begin(), prices.begin() + prices.size() / 2, prices.end());
    double median = prices[prices.size() / 2];

    vector<double> abs_devs(ticks.size());
    transform(execution::par_unseq, prices.begin(), prices.end(), abs_devs.begin(),
        [median](double p) { return abs(p - median); });
    nth_element(abs_devs.begin(), abs_devs.begin() + abs_devs.size() / 2, abs_devs.end());
    double mad = abs_devs[abs_devs.size() / 2];
    double threshold = cfg_.mad_threshold * 1.4826 * mad;

    for_each(execution::par_unseq, ticks.begin(), ticks.end(),
        [median, threshold](CleanedTick& t) {
            if (abs(t.price - median) > threshold) t.outlier = true;
        });
}

void Cleaner::flag_jump_outliers(vector<CleanedTick>& ticks) const {
    for (size_t i = 1; i < ticks.size(); ++i) {
        double prev = ticks[i - 1].price;
        double curr = ticks[i].price;
        if (prev > 0.0 && abs(curr - prev) / prev > cfg_.max_price_jump_pct) {
            ticks[i].outlier = true;
        }
    }
}

} // namespace qp::etl
