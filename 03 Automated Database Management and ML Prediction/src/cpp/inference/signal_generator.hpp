#pragma once

#include "common/types.hpp"
#include "db/connection_pool.hpp"
#include "features/feature_matrix.hpp"
#include "inference/onnx_engine.hpp"
#include <vector>

using namespace std;

namespace qp::inference {

class SignalGenerator {
public:
    SignalGenerator(OnnxEngine& engine, db::ConnectionPool& pool);

    void generate_and_store(const features::FeatureMatrix& features);

private:
    OnnxEngine& engine_;
    db::ConnectionPool& pool_;
};

} // namespace qp::inference
