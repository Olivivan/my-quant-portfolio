#include "common/config.hpp"
#include "common/logger.hpp"
#include "db/connection_pool.hpp"
#include "db/feature_repository.hpp"
#include "db/time_utils.hpp"
#include "features/feature_matrix.hpp"
#include "inference/onnx_engine.hpp"
#include "inference/signal_generator.hpp"

using namespace std;
using namespace qp;

int main(int argc, char* argv[]) {
    try {
        string config_path = (argc > 1) ? argv[1] : "config/pipeline.json";
        Config cfg(config_path);
        Logger::instance();

        inference::OnnxConfig onnx_cfg{
            .model_path = cfg.raw()["ml"]["onnx_output_path"].get<string>(),
            .intra_op_threads = cfg.raw()["inference"]["intra_op_threads"].get<int>(),
            .inter_op_threads = cfg.raw()["inference"]["inter_op_threads"].get<int>(),
            .batch_size = static_cast<size_t>(cfg.raw()["inference"]["batch_size"].get<int>())
        };
        inference::OnnxEngine engine(onnx_cfg);

        string conn_str = fmt::format(
            "host={} port={} dbname={} user={} password={}",
            cfg.db_host(), cfg.db_port(), cfg.db_name(), cfg.db_user(), cfg.db_password());

        db::ConnectionPool pool(conn_str, cfg.db_pool_size());
        inference::SignalGenerator generator(engine, pool);

        for (const auto& sym : cfg.raw()["ingestion"]["symbols"]) {
            string symbol = sym.get<string>();
            db::FeatureRepository repo(pool);
            auto rows = repo.load_features(symbol, "2024-01-01", "2026-01-01");

            features::FeatureMatrix mat;
            for (const auto& r : rows) {
                features::FeatureMatrix::Row row;
                row.time = db::parse_iso8601(r.time);
                row.symbol = r.symbol;
                row.values = r.values;
                row.target_direction = r.target_direction;
                row.target_return = r.target_return;
                mat.rows.push_back(move(row));
            }
            generator.generate_and_store(mat);
        }

        QP_INFO("Inference stage completed");
    } catch (const exception& e) {
        QP_ERROR("Inference fatal error: {}", e.what());
        return 1;
    }
    return 0;
}
