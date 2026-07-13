#pragma once

#include "db/connection_pool.hpp"
#include "features/feature_matrix.hpp"
#include <vector>

using namespace std;

namespace qp::db {

class FeatureWriter {
public:
    FeatureWriter(ConnectionPool& pool, size_t batch_size = 10000);

    void write_features(const vector<features::FeatureMatrix::Row>& rows);

private:
    ConnectionPool& pool_;
    size_t batch_size_;
};

} // namespace qp::db
