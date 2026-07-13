#pragma once

#include "common/types.hpp"
#include <vector>

using namespace std;

namespace qp::features {

struct IndicatorConfig {
    int rsi_period = 14;
    int macd_fast = 12;
    int macd_slow = 26;
    int macd_signal = 9;
    int bb_period = 20;
    double bb_std = 2.0;
};

struct Indicators {
    vector<double> returns;
    vector<double> log_returns;
    vector<double> realized_vol;
    vector<double> sma_5;
    vector<double> sma_20;
    vector<double> ema_12;
    vector<double> rsi_14;
    vector<double> macd;
    vector<double> macd_signal_line;
    vector<double> bb_position;
    vector<double> momentum_10;
    vector<double> volume_sma_10;
};

class TechnicalIndicators {
public:
    explicit TechnicalIndicators(IndicatorConfig cfg = {});
    Indicators compute(const vector<Bar>& bars) const;

private:
    IndicatorConfig cfg_;

    vector<double> sma(const vector<double>& values, int period) const;
    vector<double> ema(const vector<double>& values, int period) const;
    vector<double> rsi(const vector<double>& values, int period) const;
    vector<double> macd_line(const vector<double>& values) const;
    vector<double> bb_percent_b(const vector<double>& values, int period, double num_std) const;
};

} // namespace qp::features
