#include "utils/Logger.hpp"

#ifdef __ANDROID__
#include <android/log.h>
#endif

#include <chrono>
#include <iomanip>
#include <filesystem>

namespace Engine {

std::ofstream Logger::logFile;
bool Logger::initialized = false;

// Инициализация системы логирования с ротацией файлов
void Logger::init(const std::string& fileName) {
#ifndef __ANDROID__
    if (initialized) return;

    std::filesystem::path logPath(fileName);
    
    // создание директории для логов
    if (logPath.has_parent_path()) {
        std::filesystem::create_directories(logPath.parent_path());
    }

    // ротация: переименование текущего latest.log в previous.log
    if (std::filesystem::exists(logPath)) {
        std::filesystem::path oldLogPath = logPath;
        oldLogPath.replace_filename("previous.log");
        
        std::error_code ec;
        // удаление старого previous перед перемещением
        std::filesystem::remove(oldLogPath, ec);
        std::filesystem::rename(logPath, oldLogPath, ec);
    }

    // открытие нового файла в режиме перезаписи
    logFile.open(fileName, std::ios::out | std::ios::trunc);
    if (logFile.is_open()) {
        initialized = true;
        info("Логгер инициализирован. Текущая сессия: {}", fileName);
    }
#endif
}

// Базовый метод для вывода лога
void Logger::log(LogLevel level, std::string_view fmt, std::format_args args, const std::source_location& location) {
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm bt{};
#if defined(_MSC_VER)
    localtime_s(&bt, &timeT);
#else
    localtime_r(&timeT, &bt);
#endif

    std::string timestamp = std::format("{:02}:{:02}:{:02}.{:03}", bt.tm_hour, bt.tm_min, bt.tm_sec, ms.count());
    std::string fileInfo = std::format("{}:{}", std::filesystem::path(location.file_name()).filename().string(), location.line());
    std::string message = std::vformat(fmt, args);

#ifdef __ANDROID__
    int androidLevel = ANDROID_LOG_INFO;
    if (level == LogLevel::Debug) androidLevel = ANDROID_LOG_DEBUG;
    else if (level == LogLevel::Warning) androidLevel = ANDROID_LOG_WARN;
    else if (level == LogLevel::Error) androidLevel = ANDROID_LOG_ERROR;

    __android_log_print(androidLevel, "TowerDefence", "[%s] %s", fileInfo.c_str(), message.c_str());
#else
    std::string color = std::string(levelToColor(level));
    std::string reset = "\033[0m";
    
    std::string fullLog = std::format("{} [{:<7}] [{:<15}] {}\n", 
                                     timestamp, levelToString(level), fileInfo, message);

    std::cout << color << fullLog << reset << std::flush;

    if (initialized && logFile.is_open()) {
        logFile << fullLog << std::flush;
    }
#endif
}

// Получение строкового представления уровня
std::string_view Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO";
        case LogLevel::Warning: return "WARNING";
        case LogLevel::Error:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

// Получение цвета для консоли
std::string_view Logger::levelToColor(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:   return "\033[36m";
        case LogLevel::Info:    return "\033[32m";
        case LogLevel::Warning: return "\033[33m";
        case LogLevel::Error:   return "\033[31m";
        default:                return "\033[0m";
    }
}

} // namespace Engine