#include <gtest/gtest.h>
#include "etl/cleaner.hpp"

using namespace std;
using namespace qp;

TEST(Cleaner, FlagsJumpOutlier) {
    etl::CleaningConfig cfg{
        .winsorize_quantile = 0.0, // disable winsorize
        .mad_threshold = 100.0,    // disable MAD to isolate jump
        .max_price_jump_pct = 0.05
    };
    etl::Cleaner cleaner(cfg);

    vector<Tick> ticks;
    auto t0 = chrono::system_clock::now();
    ticks.push_back(Tick{t0, "TEST", 100.0, 10.0, Side::Buy, "test"});
    ticks.push_back(Tick{t0 + chrono::seconds(1), "TEST", 200.0, 10.0, Side::Buy, "test"}); // 100% jump

    auto cleaned = cleaner.clean(move(ticks));
    EXPECT_FALSE(cleaned[0].outlier);
    EXPECT_TRUE(cleaned[1].outlier);
}
