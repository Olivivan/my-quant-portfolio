#include "common/config.hpp"
#include "common/logger.hpp"
#include "common/types.hpp"
#include "db/connection_pool.hpp"
#include "db/feature_writer.hpp"
#include "db/timescale_writer.hpp"
#include "features/aggregator.hpp"
#include "features/feature_matrix.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

using namespace std;
using namespace qp;

namespace {

vector<Bar> load_bars_from_csv(const string& path) {
    vector<Bar> bars;
    ifstream f(path);
    if (!f.is_open()) return bars;

    string line;
    getline(f, line); // skip header
    while (getline(f, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string time_str, symbol;
        double o, h, l, c, v, vw;
        int trades;
        getline(ss, time_str, ',');
        getline(ss, symbol, ',');
        ss >> o; ss.ignore();
        ss >> h; ss.ignore();
        ss >> l; ss.ignore();
        ss >> c; ss.ignore();
        ss >> v; ss.ignore();
        ss >> vw; ss.ignore();
        ss >> trades;

        tm tm = {};
        istringstream ts(time_str);
        ts >> get_time(&tm, "%Y-%m-%d %H:%M:%S");
        if (ts.fail()) continue;

        bars.push_back(Bar{
            chrono::system_clock::from_time_t(mktime(&tm)),
            symbol, o, h, l, c, v, vw, trades});
    }
    return bars;
}

} // namespace

int main(int argc, char* argv[]) {
    try {
        string config_path = (argc > 1) ? argv[1] : "config/pipeline.json";
        Config cfg(config_path);
        Logger::instance();

        string conn_str = fmt::format(
            "host={} port={} dbname={} user={} password={}",
            cfg.db_host(), cfg.db_port(), cfg.db_name(), cfg.db_user(), cfg.db_password());

        db::ConnectionPool pool(conn_str, cfg.db_pool_size());
        db::FeatureWriter writer(pool, cfg.batch_insert_size());

        features::FeatureEngine engine(features::FeatureEngine::Config{
            .target_horizon_bars = cfg.raw()["features"]["target_horizon_bars"].get<int>(),
            .target_threshold = cfg.raw()["features"]["target_threshold"].get<double>()
        });

        filesystem::path clean_dir = cfg.clean_data_dir();
        for (const auto& entry : filesystem::directory_iterator(clean_dir)) {
            if (!entry.is_regular_file()) continue;
            auto bars = load_bars_from_csv(entry.path().string());
            if (bars.empty()) continue;
            auto mat = engine.build(bars);
            writer.write_features(mat.rows);
        }

        QP_INFO("Feature engineering stage completed");
    } catch (const exception& e) {
        QP_ERROR("Feature stage fatal error: {}", e.what());
        return 1;
    }
    return 0;
}
