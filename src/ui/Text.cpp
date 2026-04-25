#include "ui/Text.hpp"
#include "Colors.hpp"
#include <vector>

namespace UI {

// Конструктор текста инициализирует базовые параметры
Text::Text(const sf::Font& font, const std::string& utf8Text, unsigned int fontSize) {
    text = std::make_unique<sf::Text>(font);
    text->setCharacterSize(fontSize);
    text->setFillColor(Colors::Theme::TextMain);
    alignment = Align::Left;
    setText(utf8Text);
}

// Конструктор текста с размером контейнера
Text::Text(const sf::Font& font, const std::string& utf8Text, unsigned int fontSize, sf::Vector2f containerSize) {
    text = std::make_unique<sf::Text>(font);
    text->setCharacterSize(fontSize);
    text->setFillColor(Colors::Theme::TextMain);
    alignment = Align::Left;
    this->size = containerSize;
    rawString = utf8Text;
    text->setString(sf::String::fromUtf8(rawString.begin(), rawString.end()));
    applyAlignment();
}

// Изменение позиции текста
void Text::setPosition(sf::Vector2f pos) {
    position = pos;
    applyAlignment();
}

// Изменение размера
void Text::setSize(sf::Vector2f newSize) {
    size = newSize;
    applyAlignment();
}

// Изменение текстового содержимого
void Text::setText(const std::string& utf8Text) {
    rawString = utf8Text;
    applyWrapping();
}

// Изменение лимита ширины
void Text::setMaxWidth(float width) {
    maxWidth = width;
    applyWrapping();
}

// Изменение режима выравнивания
void Text::setAlignment(Align align) {
    alignment = align;
    applyAlignment();
}

// Применение смещения для выравнивания и коррекции Origin
void Text::applyAlignment() const {
    if (!text) return;
    
    sf::FloatRect bounds = text->getLocalBounds();

    // 1. Находим центр самого текста (его пикселей)
    // В SFML 3.0 используем position.x/y и size.x/y
    float originX = bounds.position.x;
    if (alignment == Align::Center) {
        originX += bounds.size.x / 2.f;
    }
    else if (alignment == Align::Right) {
        originX += bounds.size.x;
    }

    // Центрируем Origin по вертикали (середина высоты букв)
    float originY = bounds.position.y + (bounds.size.y / 2.f);

    text->setOrigin({ originX, originY });

    // 2. Устанавливаем позицию в геометрический центр контейнера 'size'
    float targetX = position.x;
    if (alignment == Align::Center) {
        targetX += size.x / 2.f;
    }
    else if (alignment == Align::Right) {
        targetX += size.x;
    }

    float targetY = position.y + (size.y / 2.f);

    text->setPosition({ targetX, targetY });
}

// Расчет переносов и обновление размеров виджета
void Text::applyWrapping() {
    if (!text) return;

    // Устанавливаем текст
    text->setString(sf::String::fromUtf8(rawString.begin(), rawString.end()));

    // Логика переноса (выполняется только если задан maxWidth)
    if (maxWidth > 0.f && !rawString.empty()) {
        sf::String utf32Text = sf::String::fromUtf8(rawString.begin(), rawString.end());
        sf::Text helper(text->getFont());
        helper.setCharacterSize(text->getCharacterSize());

        std::vector<sf::String> words;
        sf::String currentWord;
        for (char32_t c : utf32Text) {
            if (c == U' ') {
                if (!currentWord.isEmpty()) words.push_back(currentWord);
                currentWord.clear();
            }
            else currentWord += c;
        }
        if (!currentWord.isEmpty()) words.push_back(currentWord);

        sf::String result, currentLine;
        for (const auto& word : words) {
            sf::String testLine = currentLine.isEmpty() ? word : currentLine + U" " + word;
            helper.setString(testLine);
            if (helper.getLocalBounds().size.x > maxWidth) {
                result += currentLine + U"\n";
                currentLine = word;
            }
            else currentLine = testLine;
        }
        result += currentLine;
        text->setString(result);
    }

    sf::FloatRect bounds = text->getLocalBounds();

    // обновляем размеры виджета только если они еще не заданы
    if (this->size.x <= 0.f) this->size.x = bounds.size.x;
    if (this->size.y <= 0.f) {
        float fontHeight = (float)text->getCharacterSize();
        this->size.y = std::max(bounds.size.y, fontHeight);
    }

    // Если есть maxWidth, ширина виджета всегда равна ему
    if (maxWidth > 0.f) this->size.x = maxWidth;

    applyAlignment();
}

// Изменение цвета
void Text::setColor(sf::Color color) {
    if (text) text->setFillColor(color);
}

// Изменение размера шрифта
void Text::setFontSize(unsigned int fontSize) {
    if (text) text->setCharacterSize(fontSize);
    applyWrapping();
}

// Получение указателя на внутренний sf::Text
sf::Text* Text::getText() {
    return text.get();
}

// Изменение межстрочного интервала
void Text::setLineSpacing(float spacing) {
    if (text) text->setLineSpacing(spacing);
}

// Получение локальных границ
sf::FloatRect Text::getLocalBounds() const {
    if (text) return text->getLocalBounds();
    return {};
}

// Обработка событий
void Text::handleEvent(const sf::Event&, const sf::RenderWindow&, const sf::View&) {
}

// Отрисовка
void Text::render(sf::RenderWindow& window) const {
    if (!visible || !text) return;
    applyAlignment();
    window.draw(*text);
}

// Получение глобальных границ
sf::FloatRect Text::getGlobalBounds() const {
    if (text) return text->getGlobalBounds();
    return {};
}

}
