#include "tick_parser.hpp"
#include "common/logger.hpp"
#include <array>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>

using namespace std;

namespace qp::etl {

namespace {

optional<Timestamp> parse_iso_datetime(const string& s) {
    // Accept both 'T' and space as date/time separators
    string normalized = s;
    if (normalized.size() > 10 && normalized[10] == ' ') {
        normalized[10] = 'T';
    }

    tm tm = {};
    istringstream ss(normalized);
    ss >> get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (ss.fail()) return nullopt;
    auto tp = chrono::system_clock::from_time_t(mktime(&tm));

    // Parse fractional seconds if present
    if (ss.peek() == '.') {
        double frac = 0.0;
        ss >> frac;
        tp += chrono::duration_cast<chrono::system_clock::duration>(
            chrono::duration<double>(frac));
    }
    return tp;
}

Side parse_side(const string& s) {
    if (s == "1" || s == "buy" || s == "B") return Side::Buy;
    if (s == "-1" || s == "sell" || s == "S") return Side::Sell;
    return Side::Unknown;
}

} // namespace

expected<vector<Tick>, ParseError> TickParser::parse_csv(const string& file_path) const {
    ifstream file(file_path);
    if (!file.is_open()) {
        return unexpected(ParseError{"Cannot open file: " + file_path, 0});
    }

    string content((istreambuf_iterator<char>(file)),
                        istreambuf_iterator<char>());
    return parse_csv_buffer(span(content.data(), content.size()));
}

expected<vector<Tick>, ParseError> TickParser::parse_csv_buffer(span<const char> buffer) const {
    vector<Tick> ticks;
    ticks.reserve(1'000'000);

    string_view data(buffer.data(), buffer.size());
    size_t line_no = 0;
    size_t pos = 0;

    // Skip header
    auto nl = data.find('\n', pos);
    if (nl != string_view::npos) pos = nl + 1;
    ++line_no;

    while (pos < data.size()) {
        ++line_no;
        nl = data.find('\n', pos);
        string_view line = (nl == string_view::npos)
            ? data.substr(pos)
            : data.substr(pos, nl - pos);
        pos = (nl == string_view::npos) ? data.size() : nl + 1;

        if (line.empty()) continue;

        array<string_view, 6> cols{};
        size_t c = 0;
        size_t start = 0;
        for (size_t i = 0; i < line.size() && c < cols.size(); ++i) {
            if (line[i] == ',') {
                cols[c++] = line.substr(start, i - start);
                start = i + 1;
            }
        }
        if (c < cols.size() - 1) {
            return unexpected(ParseError{"Too few columns", line_no});
        }
        cols[c] = line.substr(start);

        auto time = parse_iso_datetime(string(cols[0]));
        if (!time) {
            return unexpected(ParseError{"Invalid timestamp: " + string(cols[0]), line_no});
        }

        try {
            ticks.push_back(Tick{
                .time = *time,
                .symbol = string(cols[1]),
                .price = stod(string(cols[2])),
                .size = stod(string(cols[3])),
                .side = parse_side(string(cols[4])),
                .source = string(cols[5])
            });
        } catch (const exception& e) {
            return unexpected(ParseError{string("Parse exception: ") + e.what(), line_no});
        }
    }

    QP_INFO("Parsed {} ticks", ticks.size());
    return ticks;
}

} // namespace qp::etl
