#include "common/config.hpp"
#include "common/logger.hpp"
#include "common/types.hpp"
#include "db/connection_pool.hpp"
#include "db/timescale_writer.hpp"
#include "etl/cleaner.hpp"
#include "etl/csv_writer.hpp"
#include "etl/imputer.hpp"
#include "etl/tick_parser.hpp"
#include "features/aggregator.hpp"

#include <filesystem>
#include <future>
#include <thread>
#include <vector>

using namespace std;
using namespace qp;

int main(int argc, char* argv[]) {
    try {
        string config_path = (argc > 1) ? argv[1] : "config/pipeline.json";
        string symbol_filter = (argc > 2) ? argv[2] : "";
        Config cfg(config_path);

        Logger::instance();
        QP_INFO("Starting ETL pipeline{}", symbol_filter.empty() ? "" : " for " + symbol_filter);

        string conn_str = fmt::format(
            "host={} port={} dbname={} user={} password={}",
            cfg.db_host(), cfg.db_port(), cfg.db_name(), cfg.db_user(), cfg.db_password());

        db::ConnectionPool pool(conn_str, cfg.db_pool_size());
        db::TimescaleWriter writer(pool, cfg.batch_insert_size());

        etl::CleaningConfig clean_cfg{
            .winsorize_quantile = cfg.winsorize_quantile(),
            .mad_threshold = cfg.mad_threshold(),
            .max_price_jump_pct = cfg.max_price_jump_pct()
        };
        etl::Cleaner cleaner(clean_cfg);
        etl::Imputer imputer;
        features::BarAggregator aggregator(chrono::seconds(60));

        vector<future<void>> futures;

        for (const auto& entry : filesystem::directory_iterator(cfg.raw_data_dir())) {
            if (!entry.is_regular_file()) continue;

            string file_symbol = entry.path().stem().string();
            if (!symbol_filter.empty() && file_symbol != symbol_filter) continue;

            futures.push_back(async(launch::async, [&, path = entry.path().string()] {
                etl::TickParser parser;
                auto maybe_ticks = parser.parse_csv(path);
                if (!maybe_ticks) {
                    QP_ERROR("Parse failed: {} at line {}", maybe_ticks.error().message, maybe_ticks.error().line);
                    return;
                }

                auto cleaned = cleaner.clean(move(*maybe_ticks));
                auto imputed = imputer.resample_and_impute(cleaned, chrono::seconds(1));
                writer.write_ticks(cleaned);

                auto bars = aggregator.aggregate(cleaned);
                writer.write_bars(bars);

                string clean_file = (filesystem::path(cfg.clean_data_dir()) / (filesystem::path(path).stem().string() + "_bars.csv")).string();
                etl::CsvWriter::write_bars(clean_file, bars);
            }));

            if (futures.size() >= static_cast<size_t>(cfg.thread_pool_size())) {
                for (auto& f : futures) f.get();
                futures.clear();
            }
        }

        for (auto& f : futures) f.get();
        QP_INFO("ETL pipeline completed");

    } catch (const exception& e) {
        QP_ERROR("ETL fatal error: {}", e.what());
        return 1;
    }
    return 0;
}
