#pragma once
#include <filesystem>
#include <string>

namespace PathResolver {
    // Возвращает абсолютный путь к папке ассетов (assets или data)
    std::filesystem::path getResourcesPath(const std::string& subDir);

    // Возвращает безопасный путь для записи файлов (сохранения/настройки)
    std::filesystem::path getWriteablePath(const std::string& filename);
}