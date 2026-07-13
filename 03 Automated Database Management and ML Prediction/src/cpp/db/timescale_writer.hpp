#pragma once

#include "common/types.hpp"
#include "db/connection_pool.hpp"
#include <vector>

using namespace std;

namespace qp::db {

class TimescaleWriter {
public:
    TimescaleWriter(ConnectionPool& pool, size_t batch_size = 10000);

    void write_ticks(const vector<CleanedTick>& ticks);
    void write_bars(const vector<Bar>& bars);

private:
    ConnectionPool& pool_;
    size_t batch_size_;
};

} // namespace qp::db
