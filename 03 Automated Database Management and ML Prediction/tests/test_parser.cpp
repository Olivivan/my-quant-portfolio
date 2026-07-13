#include <gtest/gtest.h>
#include "etl/tick_parser.hpp"

using namespace std;
using namespace qp;

TEST(TickParser, ParsesValidCSVBuffer) {
    string csv =
        "time,symbol,price,size,side,source\n"
        "2024-01-01T09:30:00.000,AAPL,150.25,100,1,synthetic\n"
        "2024-01-01T09:30:01.000,AAPL,150.30,200,-1,synthetic\n";

    etl::TickParser parser;
    auto result = parser.parse_csv_buffer(span(csv.data(), csv.size()));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 2);
    EXPECT_EQ(result->front().symbol, "AAPL");
}
