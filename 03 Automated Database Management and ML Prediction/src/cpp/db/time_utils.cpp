#include "time_utils.hpp"
#include <iomanip>
#include <sstream>

using namespace std;

namespace qp::db {

string to_iso8601(const chrono::system_clock::time_point& tp) {
    time_t t = chrono::system_clock::to_time_t(tp);
    tm gmt{};
#ifdef _WIN32
    gmtime_s(&gmt, &t);
#else
    gmtime_r(&t, &gmt);
#endif

    auto ms = chrono::duration_cast<chrono::milliseconds>(tp.time_since_epoch()) % 1000;

    ostringstream oss;
    oss << put_time(&gmt, "%Y-%m-%d %H:%M:%S") << "." << setw(3) << setfill('0') << ms.count();
    return oss.str();
}
chrono::system_clock::time_point parse_iso8601(const string& s) {
    tm tm = {};
    istringstream ss(s);
    ss >> get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (ss.fail()) {
        return chrono::system_clock::from_time_t(0);
    }
#ifdef _WIN32
    time_t t = _mkgmtime(&tm);
#else
    time_t t = timegm(&tm);
#endif
    if (t == -1) {
        return chrono::system_clock::from_time_t(0);
    }
    return chrono::system_clock::from_time_t(t);
}
} // namespace qp::db
