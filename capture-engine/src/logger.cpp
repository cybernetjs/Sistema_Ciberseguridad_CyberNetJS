#include "logger.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>

namespace nids {

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

void Logger::write(std::ostream& stream, const std::string& level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_r(&now_c, &tm);

    stream << "[" << std::put_time(&tm, "%H:%M:%S") << "] [" << level << "] " << message << std::endl;
}

void Logger::info(const std::string& message) { write(std::cout, "INFO", message); }
void Logger::warn(const std::string& message) { write(std::cout, "WARN", message); }
void Logger::error(const std::string& message) { write(std::cerr, "ERROR", message); }

}

