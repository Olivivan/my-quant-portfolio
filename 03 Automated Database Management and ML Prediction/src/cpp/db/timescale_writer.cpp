#include "timescale_writer.hpp"
#include "common/logger.hpp"
#include "db/time_utils.hpp"
#include <fmt/format.h>
#include <sstream>

using namespace std;

namespace qp::db {

TimescaleWriter::TimescaleWriter(ConnectionPool& pool, size_t batch_size)
    : pool_(pool), batch_size_(batch_size) {}

void TimescaleWriter::write_ticks(const vector<CleanedTick>& ticks) {
    if (ticks.empty()) return;

    for (size_t start = 0; start < ticks.size(); start += batch_size_) {
        size_t end = min(start + batch_size_, ticks.size());

        PooledConnection conn(pool_);
        pqxx::work tx(*conn);

        stringstream ss;
        ss << "INSERT INTO ticks (time, symbol, price, size, side, source) VALUES ";
        for (size_t i = start; i < end; ++i) {
            const auto& t = ticks[i];
            if (i > start) ss << ", ";
            ss << fmt::format("('{}', {}, {}, {}, {}, {})",
                to_iso8601(t.time),
                tx.quote(t.symbol),
                t.price,
                t.size,
                static_cast<int>(t.side),
                tx.quote(t.source));
        }
        tx.exec(ss.str());
        tx.commit();
    }
    QP_INFO("Wrote {} ticks to TimescaleDB", ticks.size());
}

void TimescaleWriter::write_bars(const vector<Bar>& bars) {
    if (bars.empty()) return;

    for (size_t start = 0; start < bars.size(); start += batch_size_) {
        size_t end = min(start + batch_size_, bars.size());

        PooledConnection conn(pool_);
        pqxx::work tx(*conn);

        stringstream ss;
        ss << "INSERT INTO bars_1m (time, symbol, open, high, low, close, volume, vwap, trades) VALUES ";
        for (size_t i = start; i < end; ++i) {
            const auto& b = bars[i];
            if (i > start) ss << ", ";
            ss << fmt::format("('{}', {}, {}, {}, {}, {}, {}, {}, {})",
                to_iso8601(b.time),
                tx.quote(b.symbol),
                b.open, b.high, b.low, b.close,
                b.volume, b.vwap, b.trades);
        }
        tx.exec(ss.str());
        tx.commit();
    }
    QP_INFO("Wrote {} bars to TimescaleDB", bars.size());
}

} // namespace qp::db
