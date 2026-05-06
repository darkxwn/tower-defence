#include "utils/FileReader.hpp"
#include <SFML/System/FileInputStream.hpp>

std::optional<std::string> readFile(const std::string& path) {
    sf::FileInputStream stream;
    if (!stream.open(path)) return std::nullopt;

    auto sizeOptional = stream.getSize();
    if (!sizeOptional.has_value()) return std::nullopt;
    
    std::size_t size = *sizeOptional;
    if (size == 0) return "";

    // Читаем данные
    std::string content;
    content.resize(size);

    auto bytesRead = stream.read(content.data(), size);
    if (!bytesRead) return std::nullopt;

    return content;
}