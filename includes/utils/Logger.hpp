#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <string_view>
#include <source_location>
#include <format>
#include <iostream>
#include <fstream>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС LOGGER
//
///////////////////////////////////////////////////////////////////////////

namespace Engine {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

// Вспомогательная структура для захвата строки формата и места вызова
struct LogEvent {
    std::string_view message;
    std::source_location location;

    // Конструктор захватывает место вызова в момент создания объекта
    template <typename T>
    requires std::convertible_to<T, std::string_view>
    LogEvent(const T& msg, std::source_location loc = std::source_location::current())
        : message(msg), location(loc) {}
};

class Logger {
public:
    static void init(const std::string& fileName = "latest.log");

    template <typename... Args>
    static void debug(LogEvent event, Args&&... args) {
#ifdef NDEBUG
        return;
#else
        log(LogLevel::Debug, event.message, std::make_format_args(args...), event.location);
#endif
    }

    template <typename... Args>
    static void info(LogEvent event, Args&&... args) {
        log(LogLevel::Info, event.message, std::make_format_args(args...), event.location);
    }

    template <typename... Args>
    static void warning(LogEvent event, Args&&... args) {
        log(LogLevel::Warning, event.message, std::make_format_args(args...), event.location);
    }

    template <typename... Args>
    static void error(LogEvent event, Args&&... args) {
        log(LogLevel::Error, event.message, std::make_format_args(args...), event.location);
    }

private:
    static void log(LogLevel level, std::string_view fmt, std::format_args args, const std::source_location& location);
    static std::string_view levelToString(LogLevel level);
    static std::string_view levelToColor(LogLevel level);

    static std::ofstream logFile;
    static bool initialized;
};

} // namespace Engine

#endif // LOGGER_HPP