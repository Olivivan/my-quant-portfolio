#include "feature_matrix.hpp"
#include <cmath>

using namespace std;

namespace qp::features {

namespace {

const char* g_column_names[] = {
    "returns",
    "log_returns",
    "realized_vol",
    "sma_5",
    "sma_20",
    "ema_12",
    "rsi_14",
    "macd",
    "macd_signal",
    "bb_position",
    "momentum_10",
    "volume_sma_10"
};

} // namespace

FeatureEngine::FeatureEngine(Config cfg) : cfg_(cfg) {}

const char** FeatureEngine::column_names() { return g_column_names; }
size_t FeatureEngine::num_columns() { return sizeof(g_column_names) / sizeof(g_column_names[0]); }

FeatureMatrix FeatureEngine::build(const vector<Bar>& bars) const {
    TechnicalIndicators ti;
    auto ind = ti.compute(bars);
    const size_t n = bars.size();

    FeatureMatrix mat;
    mat.rows.reserve(n);

    int h = cfg_.target_horizon_bars;
    double thresh = cfg_.target_threshold;

    for (size_t i = 0; i < n; ++i) {
        FeatureMatrix::Row row;
        row.time = bars[i].time;
        row.symbol = bars[i].symbol;
        row.values = {
            ind.returns[i],
            ind.log_returns[i],
            ind.realized_vol[i],
            ind.sma_5[i],
            ind.sma_20[i],
            ind.ema_12[i],
            ind.rsi_14[i],
            ind.macd[i],
            ind.macd_signal_line[i],
            ind.bb_position[i],
            ind.momentum_10[i],
            ind.volume_sma_10[i]
        };

        if (i + h < n && bars[i].close != 0.0) {
            double fwd_ret = (bars[i + h].close - bars[i].close) / bars[i].close;
            row.target_return = fwd_ret;
            if (fwd_ret > thresh) row.target_direction = 1;
            else if (fwd_ret < -thresh) row.target_direction = -1;
            else row.target_direction = 0;
        } else {
            row.target_direction = 0;
            row.target_return = 0.0;
        }
        mat.rows.push_back(move(row));
    }
    return mat;
}

} // namespace qp::features
