#include "feature_writer.hpp"
#include "common/logger.hpp"
#include "db/time_utils.hpp"
#include <fmt/format.h>
#include <sstream>

using namespace std;

namespace qp::db {

FeatureWriter::FeatureWriter(ConnectionPool& pool, size_t batch_size)
    : pool_(pool), batch_size_(batch_size) {}

void FeatureWriter::write_features(const vector<features::FeatureMatrix::Row>& rows) {
    if (rows.empty()) return;

    for (size_t start = 0; start < rows.size(); start += batch_size_) {
        size_t end = min(start + batch_size_, rows.size());

        PooledConnection conn(pool_);
        pqxx::work tx(*conn);

        stringstream ss;
        ss << "INSERT INTO features (time, symbol, returns, log_returns, realized_vol, "
              "sma_5, sma_20, ema_12, rsi_14, macd, macd_signal, bb_position, "
              "momentum_10, volume_sma_10, target_direction, target_return) VALUES ";

        for (size_t i = start; i < end; ++i) {
            const auto& r = rows[i];
            if (i > start) ss << ", ";
            ss << fmt::format("('{}', {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {})",
                to_iso8601(r.time),
                tx.quote(r.symbol),
                r.values[0], r.values[1], r.values[2], r.values[3],
                r.values[4], r.values[5], r.values[6], r.values[7],
                r.values[8], r.values[9], r.values[10], r.values[11],
                r.target_direction, r.target_return);
        }
        tx.exec(ss.str());
        tx.commit();
    }
    QP_INFO("Wrote {} feature rows to TimescaleDB", rows.size());
}

} // namespace qp::db
