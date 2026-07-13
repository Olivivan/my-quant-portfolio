#include "signal_generator.hpp"
#include "common/logger.hpp"
#include "db/time_utils.hpp"
#include <fmt/format.h>
#include <pqxx/pqxx>
#include <sstream>

using namespace std;
using namespace qp::db;

namespace qp::inference {

SignalGenerator::SignalGenerator(OnnxEngine& engine, db::ConnectionPool& pool)
    : engine_(engine), pool_(pool) {}

void SignalGenerator::generate_and_store(const features::FeatureMatrix& features) {
    if (features.rows.empty()) return;

    vector<vector<float>> float_features(features.rows.size());
    for (size_t i = 0; i < features.rows.size(); ++i) {
        float_features[i].reserve(features.rows[i].values.size());
        for (double v : features.rows[i].values) float_features[i].push_back(static_cast<float>(v));
    }

    auto probs = engine_.predict_proba_up(float_features);
    auto preds = engine_.predict_direction(float_features);

    db::PooledConnection conn(pool_);
    pqxx::work tx(*conn);

    stringstream ss;
    ss << "INSERT INTO signals (time, symbol, model_name, prediction, probability) VALUES ";
    const string model_name = "lgbm_direction";

    for (size_t i = 0; i < features.rows.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << fmt::format("('{}', {}, {}, {}, {})",
            to_iso8601(features.rows[i].time),
            tx.quote(features.rows[i].symbol),
            tx.quote(model_name),
            preds[i],
            probs[i]);
    }
    tx.exec(ss.str());
    tx.commit();

    QP_INFO("Generated and stored {} signals", features.rows.size());
}

} // namespace qp::inference
