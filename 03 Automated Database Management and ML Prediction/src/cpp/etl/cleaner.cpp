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
    if (cfg_.winsorize_quantile <= 0.0) return;

    // Winsorize on log-prices so the bounds are multiplicative and
    // scale-invariant across long price histories.
    vector<double> log_prices;
    log_prices.reserve(ticks.size());
    for (const auto& t : ticks) {
        log_prices.push_back(t.price > 0.0 ? log(t.price) : 0.0);
    }

    sort(log_prices.begin(), log_prices.end());
    size_t n = log_prices.size();
    double lower_log = log_prices[static_cast<size_t>(cfg_.winsorize_quantile * n)];
    double upper_log = log_prices[static_cast<size_t>((1.0 - cfg_.winsorize_quantile) * n)];
    double lower = exp(lower_log);
    double upper = exp(upper_log);

    for_each(execution::par_unseq, ticks.begin(), ticks.end(),
        [lower, upper](CleanedTick& t) {
            if (t.price < lower) { t.price = lower; t.outlier = true; }
            if (t.price > upper) { t.price = upper; t.outlier = true; }
        });
}

void Cleaner::flag_mad_outliers(vector<CleanedTick>& ticks) const {
    if (ticks.size() < 2) return;

    // Compute log-returns so the statistic is scale-invariant.  This avoids
    // flagging all modern prices as outliers when a symbol has appreciated
    // over decades (e.g. daily close data from Yahoo Finance MAX history).
    vector<double> log_rets;
    log_rets.reserve(ticks.size());
    double prev_price = ticks.front().price;
    for (size_t i = 1; i < ticks.size(); ++i) {
        double curr = ticks[i].price;
        if (prev_price > 0.0 && curr > 0.0) {
            log_rets.push_back(log(curr / prev_price));
        }
        prev_price = curr;
    }
    if (log_rets.empty()) return;

    auto mid = log_rets.begin() + log_rets.size() / 2;
    nth_element(log_rets.begin(), mid, log_rets.end());
    double median = *mid;

    vector<double> abs_devs(log_rets.size());
    transform(execution::par_unseq, log_rets.begin(), log_rets.end(), abs_devs.begin(),
        [median](double lr) { return abs(lr - median); });
    nth_element(abs_devs.begin(), abs_devs.begin() + abs_devs.size() / 2, abs_devs.end());
    double mad = abs_devs[abs_devs.size() / 2];
    double threshold = cfg_.mad_threshold * 1.4826 * mad;

    // Mark the *later* tick of each extreme log-return pair as the outlier.
    prev_price = ticks.front().price;
    size_t ret_idx = 0;
    for (size_t i = 1; i < ticks.size(); ++i) {
        double curr = ticks[i].price;
        if (prev_price > 0.0 && curr > 0.0 && ret_idx < log_rets.size()) {
            if (abs(log_rets[ret_idx] - median) > threshold) {
                ticks[i].outlier = true;
            }
            ++ret_idx;
        }
        prev_price = curr;
    }
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
