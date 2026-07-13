#include "feature_repository.hpp"
#include "common/logger.hpp"
#include "db/time_utils.hpp"
#include <fmt/format.h>
#include <sstream>

using namespace std;

namespace qp::db {

FeatureRepository::FeatureRepository(ConnectionPool& pool) : pool_(pool) {}

vector<FeatureRow> FeatureRepository::load_features(const string& symbol,
                                                         const string& start,
                                                         const string& end) {
    PooledConnection conn(pool_);
    pqxx::work tx(*conn);

    auto result = tx.exec_params(
        "SELECT time, symbol, returns, log_returns, realized_vol, sma_5, sma_20, "
        "ema_12, rsi_14, macd, macd_signal, bb_position, momentum_10, volume_sma_10, "
        "target_direction, target_return "
        "FROM features WHERE symbol = $1 AND time BETWEEN $2 AND $3 ORDER BY time",
        symbol, start, end);

    vector<FeatureRow> rows;
    rows.reserve(result.size());
    for (const auto& r : result) {
        FeatureRow row;
        row.time = r[0].as<string>();
        row.symbol = r[1].as<string>();
        for (int i = 2; i < 14; ++i) {
            row.values.push_back(r[i].is_null() ? 0.0 : r[i].as<double>());
        }
        row.target_direction = r[14].is_null() ? 0 : r[14].as<int>();
        row.target_return = r[15].is_null() ? 0.0 : r[15].as<double>();
        rows.push_back(move(row));
    }
    QP_INFO("Loaded {} feature rows for {}", rows.size(), symbol);
    return rows;
}

void FeatureRepository::write_predictions(const string& symbol,
                                          const string& model_name,
                                          const vector<FeatureRow>& rows) {
    PooledConnection conn(pool_);
    pqxx::work tx(*conn);

    stringstream ss;
    ss << "INSERT INTO signals (time, symbol, model_name, prediction, probability) VALUES ";
    for (size_t i = 0; i < rows.size(); ++i) {
        const auto& r = rows[i];
        if (i > 0) ss << ", ";
        ss << fmt::format("('{}', {}, {}, {}, {})",
            r.time, tx.quote(symbol), tx.quote(model_name),
            r.target_direction, r.target_return);
    }
    tx.exec(ss.str());
    tx.commit();
    QP_INFO("Wrote {} predictions for {}", rows.size(), symbol);
}

} // namespace qp::db
