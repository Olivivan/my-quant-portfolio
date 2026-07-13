#pragma once

#include "common/types.hpp"
#include <vector>

using namespace std;

namespace qp::etl {

struct CleaningConfig {
    double winsorize_quantile = 0.01;
    double mad_threshold = 5.0;
    double max_price_jump_pct = 0.05;
};

class Cleaner {
public:
    explicit Cleaner(CleaningConfig cfg);

    vector<CleanedTick> clean(vector<Tick>&& ticks) const;

private:
    CleaningConfig cfg_;

    void winsorize_prices(vector<CleanedTick>& ticks) const;
    void flag_mad_outliers(vector<CleanedTick>& ticks) const;
    void flag_jump_outliers(vector<CleanedTick>& ticks) const;
};

} // namespace qp::etl
