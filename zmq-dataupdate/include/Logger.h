#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3
};

class Logger {
private:
    std::ofstream logFile;
    std::mutex logMutex;
    LogLevel currentLevel;
    std::string getCurrentTime();
    std::string levelToString(LogLevel level);

public:
    Logger(const std::string& filename = "zmq_subscriber.log", LogLevel level = LogLevel::INFO);
    ~Logger();

    void setLogLevel(LogLevel level) { currentLevel = level; }
    
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    
    void log(LogLevel level, const std::string& message);
};

#endif // LOGGER_H 