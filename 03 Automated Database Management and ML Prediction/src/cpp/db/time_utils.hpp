#pragma once

#include "common/types.hpp"
#include <chrono>
#include <string>

using namespace std;

namespace qp::db {

// Format timestamp as ISO-8601 string suitable for PostgreSQL TIMESTAMPTZ
string to_iso8601(const chrono::system_clock::time_point& tp);

// Parse ISO-8601 string of the form "YYYY-MM-DD HH:MM:SS[.fff]" into a time_point.
// Returns epoch on parse failure.
chrono::system_clock::time_point parse_iso8601(const string& s);

} // namespace qp::db
