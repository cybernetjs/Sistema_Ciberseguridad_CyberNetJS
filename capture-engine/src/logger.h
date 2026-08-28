#pragma once

#include <mutex>
#include <string>

namespace nids {

class Logger {
public:
    static Logger& instance();

    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger() = default;
    void write(std::ostream& stream, const std::string& level, const std::string& message);

    std::mutex mutex_;
};

}

