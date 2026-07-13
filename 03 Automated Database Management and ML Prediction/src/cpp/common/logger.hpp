#pragma once

#include <memory>
#include <spdlog/spdlog.h>

using namespace std;

namespace qp {

class Logger {
public:
    static Logger& instance();

    shared_ptr<spdlog::logger> logger() { return logger_; }

private:
    Logger();
    shared_ptr<spdlog::logger> logger_;
};

#define QP_INFO(...) qp::Logger::instance().logger()->info(__VA_ARGS__)
#define QP_WARN(...) qp::Logger::instance().logger()->warn(__VA_ARGS__)
#define QP_ERROR(...) qp::Logger::instance().logger()->error(__VA_ARGS__)

} // namespace qp
