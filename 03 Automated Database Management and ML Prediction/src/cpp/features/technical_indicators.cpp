#include "technical_indicators.hpp"
#include <cmath>
#include <numeric>

using namespace std;

namespace qp::features {

TechnicalIndicators::TechnicalIndicators(IndicatorConfig cfg) : cfg_(cfg) {}

Indicators TechnicalIndicators::compute(const vector<Bar>& bars) const {
    const size_t n = bars.size();
    vector<double> prices(n);
    vector<double> volumes(n);
    for (size_t i = 0; i < n; ++i) {
        prices[i] = bars[i].close;
        volumes[i] = bars[i].volume;
    }

    Indicators ind;
    ind.returns.resize(n, 0.0);
    ind.log_returns.resize(n, 0.0);
    for (size_t i = 1; i < n; ++i) {
        if (prices[i - 1] != 0.0) {
            ind.returns[i] = (prices[i] - prices[i - 1]) / prices[i - 1];
            ind.log_returns[i] = log(prices[i] / prices[i - 1]);
        }
    }

    ind.realized_vol.resize(n, 0.0);
    int vol_window = 20;
    for (size_t i = vol_window; i < n; ++i) {
        double mean = accumulate(ind.log_returns.begin() + i - vol_window,
                                      ind.log_returns.begin() + i, 0.0) / vol_window;
        double var = 0.0;
        for (size_t j = i - vol_window; j < i; ++j) {
            var += (ind.log_returns[j] - mean) * (ind.log_returns[j] - mean);
        }
        ind.realized_vol[i] = sqrt(var / vol_window) * sqrt(252.0 * 390.0); // Annualized intraday
    }

    ind.sma_5 = sma(prices, 5);
    ind.sma_20 = sma(prices, 20);
    ind.ema_12 = ema(prices, cfg_.macd_fast);
    ind.rsi_14 = rsi(prices, cfg_.rsi_period);
    ind.macd = macd_line(prices);
    ind.macd_signal_line = ema(ind.macd, cfg_.macd_signal);
    ind.bb_position = bb_percent_b(prices, cfg_.bb_period, cfg_.bb_std);

    ind.momentum_10.resize(n, 0.0);
    for (size_t i = 10; i < n; ++i) {
        if (prices[i - 10] != 0.0) ind.momentum_10[i] = prices[i] / prices[i - 10] - 1.0;
    }

    ind.volume_sma_10 = sma(volumes, 10);
    return ind;
}

vector<double> TechnicalIndicators::sma(const vector<double>& values, int period) const {
    vector<double> out(values.size(), 0.0);
    if (period <= 0 || values.size() < static_cast<size_t>(period)) return out;
    double sum = accumulate(values.begin(), values.begin() + period, 0.0);
    out[period - 1] = sum / period;
    for (size_t i = period; i < values.size(); ++i) {
        sum += values[i] - values[i - period];
        out[i] = sum / period;
    }
    return out;
}

vector<double> TechnicalIndicators::ema(const vector<double>& values, int period) const {
    vector<double> out(values.size(), 0.0);
    if (period <= 0 || values.empty()) return out;
    double mult = 2.0 / (period + 1);
    out[0] = values[0];
    for (size_t i = 1; i < values.size(); ++i) {
        out[i] = values[i] * mult + out[i - 1] * (1.0 - mult);
    }
    return out;
}

vector<double> TechnicalIndicators::rsi(const vector<double>& values, int period) const {
    vector<double> out(values.size(), 50.0);
    if (period <= 0 || values.size() < static_cast<size_t>(period + 1)) return out;

    double avg_gain = 0.0, avg_loss = 0.0;
    for (size_t i = 1; i <= static_cast<size_t>(period); ++i) {
        double diff = values[i] - values[i - 1];
        if (diff > 0) avg_gain += diff;
        else avg_loss += -diff;
    }
    avg_gain /= period;
    avg_loss /= period;

    if (avg_loss == 0.0) out[period] = 100.0;
    else {
        double rs = avg_gain / avg_loss;
        out[period] = 100.0 - (100.0 / (1.0 + rs));
    }

    for (size_t i = period + 1; i < values.size(); ++i) {
        double diff = values[i] - values[i - 1];
        double gain = diff > 0.0 ? diff : 0.0;
        double loss = diff < 0.0 ? -diff : 0.0;
        avg_gain = (avg_gain * (period - 1) + gain) / period;
        avg_loss = (avg_loss * (period - 1) + loss) / period;
        if (avg_loss == 0.0) out[i] = 100.0;
        else {
            double rs = avg_gain / avg_loss;
            out[i] = 100.0 - (100.0 / (1.0 + rs));
        }
    }
    return out;
}

vector<double> TechnicalIndicators::macd_line(const vector<double>& values) const {
    auto fast = ema(values, cfg_.macd_fast);
    auto slow = ema(values, cfg_.macd_slow);
    vector<double> out(values.size(), 0.0);
    for (size_t i = 0; i < values.size(); ++i) {
        out[i] = fast[i] - slow[i];
    }
    return out;
}

vector<double> TechnicalIndicators::bb_percent_b(const vector<double>& values, int period, double num_std) const {
    vector<double> out(values.size(), 0.5);
    if (period <= 0 || values.size() < static_cast<size_t>(period)) return out;

    for (size_t i = period - 1; i < values.size(); ++i) {
        double mean = accumulate(values.begin() + i - period + 1,
                                      values.begin() + i + 1, 0.0) / period;
        double var = 0.0;
        for (size_t j = i - period + 1; j <= i; ++j) {
            var += (values[j] - mean) * (values[j] - mean);
        }
        double stddev = sqrt(var / period);
        double upper = mean + num_std * stddev;
        double lower = mean - num_std * stddev;
        double range = upper - lower;
        if (range != 0.0) out[i] = (values[i] - lower) / range;
    }
    return out;
}

} // namespace qp::features
