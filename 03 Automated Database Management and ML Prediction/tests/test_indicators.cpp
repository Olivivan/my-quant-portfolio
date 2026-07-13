#include <gtest/gtest.h>
#include "features/technical_indicators.hpp"

using namespace std;
using namespace qp;

TEST(Indicators, SMAReturnsCorrectLength) {
    features::TechnicalIndicators ti;
    vector<Bar> bars(50, Bar{});
    auto now = chrono::system_clock::now();
    for (size_t i = 0; i < bars.size(); ++i) {
        bars[i] = Bar{now + chrono::seconds(i), "TEST", 100.0 + i, 100.0 + i, 100.0 + i, 100.0 + i, 100.0, 100.0, 10};
    }
    auto ind = ti.compute(bars);
    EXPECT_EQ(ind.sma_5.size(), bars.size());
    EXPECT_EQ(ind.rsi_14.size(), bars.size());
}
