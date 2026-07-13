#pragma once

#include "db/connection_pool.hpp"
#include <string>
#include <vector>

using namespace std;

namespace qp::db {

struct FeatureRow {
    string time;
    string symbol;
    vector<double> values;
    int target_direction;
    double target_return;
};

class FeatureRepository {
public:
    explicit FeatureRepository(ConnectionPool& pool);

    vector<FeatureRow> load_features(const string& symbol,
                                          const string& start,
                                          const string& end);

    void write_predictions(const string& symbol,
                           const string& model_name,
                           const vector<FeatureRow>& rows);

private:
    ConnectionPool& pool_;
};

} // namespace qp::db
