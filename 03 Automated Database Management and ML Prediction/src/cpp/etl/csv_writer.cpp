#include "csv_writer.hpp"
#include "common/logger.hpp"
#include "db/time_utils.hpp"
#include <filesystem>
#include <fstream>
#include <iomanip>

using namespace std;

namespace qp::etl {

void CsvWriter::write_ticks(const string& path, const vector<CleanedTick>& ticks) {
    filesystem::create_directories(filesystem::path(path).parent_path());
    ofstream f(path);
    if (!f.is_open()) {
        QP_ERROR("Cannot write ticks to {}", path);
        return;
    }
    f << "time,symbol,price,size,side,source,outlier,imputed\n";
    f << fixed << setprecision(4);
    for (const auto& t : ticks) {
        f << db::to_iso8601(t.time) << ","
          << t.symbol << ","
          << t.price << ","
          << t.size << ","
          << static_cast<int>(t.side) << ","
          << t.source << ","
          << t.outlier << ","
          << t.imputed << "\n";
    }
    QP_INFO("Wrote {} ticks to {}", ticks.size(), path);
}

void CsvWriter::write_bars(const string& path, const vector<Bar>& bars) {
    filesystem::create_directories(filesystem::path(path).parent_path());
    ofstream f(path);
    if (!f.is_open()) {
        QP_ERROR("Cannot write bars to {}", path);
        return;
    }
    f << "time,symbol,open,high,low,close,volume,vwap,trades\n";
    f << fixed << setprecision(4);
    for (const auto& b : bars) {
        f << db::to_iso8601(b.time) << ","
          << b.symbol << ","
          << b.open << ","
          << b.high << ","
          << b.low << ","
          << b.close << ","
          << b.volume << ","
          << b.vwap << ","
          << b.trades << "\n";
    }
    QP_INFO("Wrote {} bars to {}", bars.size(), path);
}

} // namespace qp::etl
