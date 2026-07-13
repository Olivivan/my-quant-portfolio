#pragma once

#include "common/types.hpp"
#include <string>
#include <vector>

using namespace std;

namespace qp::etl {

class CsvWriter {
public:
    static void write_ticks(const string& path, const vector<CleanedTick>& ticks);
    static void write_bars(const string& path, const vector<Bar>& bars);
};

} // namespace qp::etl
