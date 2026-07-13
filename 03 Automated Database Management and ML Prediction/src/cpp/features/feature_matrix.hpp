#pragma once

#include "common/types.hpp"
#include "features/technical_indicators.hpp"
#include <vector>

using namespace std;

namespace qp::features {

struct FeatureMatrix {
    struct Row {
        Timestamp time;
        string symbol;
        vector<double> values;
        int target_direction;
        double target_return;
    };

    vector<Row> rows;
};

class FeatureEngine {
public:
    struct Config {
        int target_horizon_bars = 5;
        double target_threshold = 0.001;
    };

    explicit FeatureEngine(Config cfg);
    FeatureMatrix build(const vector<Bar>& bars) const;

    static const char** column_names();
    static size_t num_columns();

private:
    Config cfg_;
};

} // namespace qp::features
