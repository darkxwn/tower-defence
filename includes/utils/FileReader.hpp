#pragma once
#include <string>
#include <optional>

// Читает файл в строку через SFML (кроссплатформенно)
std::optional<std::string> readFile(const std::string& path);
