#pragma once

#include "common/types.hpp"
#include <chrono>
#include <vector>

using namespace std;

namespace qp::etl {

class Imputer {
public:
    struct ImputedTick {
        Timestamp time;
        double price;
        double size;
        bool generated;
    };

    // Resample ticks to a regular grid (e.g., every second) using forward-fill + interpolation.
    vector<ImputedTick> resample_and_impute(
        const vector<CleanedTick>& ticks,
        chrono::milliseconds interval) const;
};

} // namespace qp::etl
