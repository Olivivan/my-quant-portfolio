#pragma once

#include "common/types.hpp"
#include <chrono>
#include <vector>

using namespace std;

namespace qp::features {

class BarAggregator {
public:
    explicit BarAggregator(chrono::seconds interval);

    vector<Bar> aggregate(const vector<CleanedTick>& ticks) const;

private:
    chrono::seconds interval_;
};

} // namespace qp::features
