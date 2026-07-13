#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

using namespace std;

namespace qp {

using Timestamp = chrono::system_clock::time_point;

enum class Side : int8_t {
    Sell = -1,
    Unknown = 0,
    Buy = 1
};

struct Tick {
    Timestamp time;
    string symbol;
    double price;
    double size;
    Side side;
    string source;
};

struct Bar {
    Timestamp time;
    string symbol;
    double open;
    double high;
    double low;
    double close;
    double volume;
    double vwap;
    int32_t trades;
};

struct CleanedTick : Tick {
    bool outlier;
    bool imputed;
};

using FeatureVector = vector<double>;

} // namespace qp
