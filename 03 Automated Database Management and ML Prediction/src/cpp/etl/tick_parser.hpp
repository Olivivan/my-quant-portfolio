#pragma once

#include "common/types.hpp"
#include <expected>
#include <span>
#include <string>
#include <vector>

using namespace std;

namespace qp::etl {

struct ParseError {
    string message;
    size_t line;
};

class TickParser {
public:
    // Expected CSV columns: time,symbol,price,size,side,source
    expected<vector<Tick>, ParseError> parse_csv(const string& file_path) const;
    expected<vector<Tick>, ParseError> parse_csv_buffer(span<const char> buffer) const;
};

} // namespace qp::etl
