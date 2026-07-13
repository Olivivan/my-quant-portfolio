#include "timescale_writer.hpp"
#include "common/logger.hpp"
#include "db/time_utils.hpp"
#include <cctype>
#include <cstdint>
#include <fmt/format.h>
#include <sstream>
#include <unordered_map>

using namespace std;

namespace qp::db {

namespace {

string tick_table_name(const string& symbol) {
    string name = "ticks_";
    for (char c : symbol) {
        if (isalnum(static_cast<unsigned char>(c))) {
            name += static_cast<char>(tolower(static_cast<unsigned char>(c)));
        } else {
            name += '_';
        }
    }
    return name;
}

void ensure_tick_table(pqxx::work& tx, const string& table_name) {
    auto r = tx.exec1(fmt::format(
        "SELECT EXISTS (SELECT 1 FROM pg_tables WHERE schemaname = 'public' AND tablename = '{}')",
        table_name));
    if (r[0].as<bool>()) return;

    tx.exec(fmt::format(
        "CREATE TABLE {} ("
        "time TIMESTAMPTZ NOT NULL, "
        "symbol TEXT NOT NULL, "
        "price DOUBLE PRECISION NOT NULL, "
        "size DOUBLE PRECISION NOT NULL, "
        "side SMALLINT, "
        "source TEXT, "
        "received_at TIMESTAMPTZ DEFAULT NOW()"
        ")",
        table_name));
    tx.exec(fmt::format(
        "CREATE INDEX idx_{}_time ON {} (time DESC)",
        table_name, table_name));
}

} // namespace

TimescaleWriter::TimescaleWriter(ConnectionPool& pool, size_t batch_size)
    : pool_(pool), batch_size_(batch_size) {}

void TimescaleWriter::write_ticks(const vector<CleanedTick>& ticks) {
    if (ticks.empty()) return;

    // Group ticks by symbol so each symbol lands in its own table.
    unordered_map<string, vector<const CleanedTick*>> by_symbol;
    for (const auto& t : ticks) {
        by_symbol[t.symbol].push_back(&t);
    }

    for (const auto& [symbol, ptrs] : by_symbol) {
        const string table = tick_table_name(symbol);

        for (size_t start = 0; start < ptrs.size(); start += batch_size_) {
            size_t end = min(start + batch_size_, ptrs.size());

            PooledConnection conn(pool_);
            pqxx::work tx(*conn);
            ensure_tick_table(tx, table);

            stringstream ss;
            ss << "INSERT INTO " << table << " (time, symbol, price, size, side, source) VALUES ";
            for (size_t i = start; i < end; ++i) {
                const auto& t = *ptrs[i];
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
    }
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
