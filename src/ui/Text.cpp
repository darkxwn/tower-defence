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

// Изменение позиции текста
void Text::setPosition(sf::Vector2f pos) {
    position = pos;
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
    
    // ВАЖНО: устанавливаем Origin в позицию bounds, чтобы (0,0) текста 
    // совпадал с реальным началом отрисовки букв. Это убирает "сползание" вниз.
    text->setOrigin(bounds.position);

    float offsetX = 0.f;
    if (alignment == Align::Center) {
        offsetX = -bounds.size.x / 2.f;
    }
    else if (alignment == Align::Right) {
        offsetX = -bounds.size.x;
    }

    text->setPosition({ position.x + offsetX, position.y });
}

// Расчет переносов и обновление размеров виджета
void Text::applyWrapping() {
    if (!text) return;

    if (maxWidth <= 0.f || rawString.empty()) {
        text->setString(sf::String::fromUtf8(rawString.begin(), rawString.end()));
        sf::FloatRect bounds = text->getLocalBounds();
        this->size = bounds.size;
        applyAlignment();
        return;
    }

    sf::String utf32Text = sf::String::fromUtf8(rawString.begin(), rawString.end());
    sf::Text helper(text->getFont());
    helper.setCharacterSize(text->getCharacterSize());

    std::vector<sf::String> words;
    sf::String currentWord;
    for (char32_t c : utf32Text) {
        if (c == U' ') {
            if (!currentWord.isEmpty()) words.push_back(currentWord);
            currentWord.clear();
        } else {
            currentWord += c;
        }
    }
    if (!currentWord.isEmpty()) words.push_back(currentWord);

    sf::String result, currentLine;
    for (const auto& word : words) {
        sf::String testLine = currentLine.isEmpty() ? word : currentLine + U" " + word;
        helper.setString(testLine);
        if (helper.getLocalBounds().size.x > maxWidth) {
            result += currentLine + U"\n";
            currentLine = word;
        } else {
            currentLine = testLine;
        }
    }
    result += currentLine;
    text->setString(result);
    
    sf::FloatRect bounds = text->getLocalBounds();
    this->size = bounds.size;
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
