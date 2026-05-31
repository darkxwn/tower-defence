#include "utils/PathResolver.hpp"
#include <cstdlib>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#include <limits.h>
#endif

namespace PathResolver {

// Вспомогательная функция получения директории, где лежит сам .exe / бинарник
std::filesystem::path getExecutableDir() {
#if defined(__ANDROID__)
    return ""; // На Android пути обрабатываются иначе через AssetManager
#elif defined(_WIN32)
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    return std::filesystem::path(path).parent_path();
#elif defined(__APPLE__)
    char path[1024];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        return std::filesystem::path(path).parent_path();
    }
#elif defined(__linux__)
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count != -1) {
        return std::filesystem::path(std::string(result, count)).parent_path();
    }
#endif
    return std::filesystem::current_path();
}

std::filesystem::path getResourcesPath(const std::string& subDir) {
#if defined(__ANDROID__)
    // В SFML 3 на Android ассеты читаются напрямую из APK (используем относительный путь)
    return std::filesystem::path(subDir);
#endif

    auto exeDir = getExecutableDir();

    // 1. Проверяем локальный путь (для разработки: папка лежит прямо рядом с .exe)
    auto localPath = exeDir / subDir;
    if (std::filesystem::exists(localPath)) {
        return localPath;
    }

    // 2. Проверяем путь macOS Bundle (Resources/assets)
    auto macPath = exeDir / "../Resources" / subDir;
    if (std::filesystem::exists(macPath)) {
        return macPath;
    }

    // 3. Проверяем глобальный путь Linux (после установки пакета)
    // Если бинарник в /usr/bin, то ресурсы в /usr/share/tower-defence/
    auto linuxPath = exeDir / "../share/tower-defence" / subDir;
    if (std::filesystem::exists(linuxPath)) {
        return linuxPath;
    }

    // Фолбек: если ничего не найдено, надеемся на текущую рабочую директорию
    return std::filesystem::path(subDir);
}

std::filesystem::path getWriteablePath(const std::string& filename) {
#if defined(__ANDROID__)
    // На Android пишем в локальное хранилище приложения
    return std::filesystem::path(filename);
#elif defined(_WIN32)
    // %APPDATA%/tower-defence/filename
    const char* appData = std::getenv("APPDATA");
    if (appData) {
        auto dir = std::filesystem::path(appData) / "tower-defence";
        std::filesystem::create_directories(dir);
        return dir / filename;
    }
#elif defined(__APPLE__)
    // ~/Library/Application Support/tower-defence/filename
    const char* home = std::getenv("HOME");
    if (home) {
        auto dir = std::filesystem::path(home) / "Library/Application Support/tower-defence";
        std::filesystem::create_directories(dir);
        return dir / filename;
    }
#elif defined(__linux__)
    // ~/.config/tower-defence/filename (стандарт XDG)
    const char* home = std::getenv("HOME");
    if (home) {
        auto dir = std::filesystem::path(home) / ".config" / "tower-defence";
        std::filesystem::create_directories(dir);
        return dir / filename;
    }
#endif
    return std::filesystem::path(filename); // Фолбек на локальную запись
}

} // namespace PathResolver