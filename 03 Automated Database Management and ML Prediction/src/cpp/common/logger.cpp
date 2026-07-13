#include "logger.hpp"
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

namespace qp {

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::Logger() {
    fs::create_directories("logs");

    auto console = make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file = make_shared<spdlog::sinks::basic_file_sink_mt>("logs/pipeline.log", true);

    logger_ = make_shared<spdlog::logger>("quant_pipeline",
        spdlog::sinks_init_list{console, file});
    logger_->set_level(spdlog::level::info);
    logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
}

} // namespace qp
