#pragma once

#include <iostream>
#include <llvm/IR/Value.h>

#define MAX_BUFFER_SIZE 256

#define STR(format, ...)                                                                                     \
    [](const char *fmt, auto... args) {                                                                      \
        char buffer[MAX_BUFFER_SIZE];                                                                        \
        std::snprintf(buffer, sizeof(buffer), fmt, args...);                                                 \
        return std::string(buffer);                                                                          \
    }(format, __VA_ARGS__)

#define JLANG_ERROR(MSG) LogErrorV(MSG)

#define LOG(severity, message) jlang::Logger::log(severity, message, __FILE__, __LINE__)

#define JLANG_DEBUG(message) LOG("DEBUG", message)
#define JLANG_LOG_INFO(message) LOG("INFO", message)
#define JLANG_LOG_WARN(message) LOG("WARN", message)
#define JLANG_LOG_ERROR(message) LOG("ERROR", message)

inline llvm::Value *LogErrorV(const char *message)
{
    std::cerr << "JLANG ERROR: " << message << std::endl;
    return nullptr;
}

namespace jlang
{

class Logger
{
  public:
    Logger() = delete;

    static void log(const std::string &severity, const std::string &message, const char *file,
                    uint32_t lineNumber)
    {
        std::ofstream log("log.txt");

        if (!log.is_open())
        {
            std::cerr << "No can do for logging!\r\n";
            return;
        }

        putLogMessage(log, message, severity, lineNumber, file);

        log.close();
    }

  private:
    static void putLogMessage(std::ofstream &log, const std::string &message, const std::string &severity,
                              uint32_t lineNumber, const char *file)
    {
        log << "[";
        log << " JLANG ";
        log << "] ";

        std::ostringstream location;
        location << "[" << file << ":" << lineNumber << "]";

        log << location.str();

        size_t padding = 74 - location.str().length();
        if (padding > 0)
        {
            log << std::setw(padding) << " ";
        }

        log << severity << ": " << message << "\r\n";
    }
};

} // namespace jlang