#include "Label.hpp"

Label::Label(const sf::Font& font, const std::string& utf8Text, unsigned int fontSize) 
    : m_text(font) // Передаем шрифт в конструктор sf::Text
{
    m_text.setCharacterSize(fontSize);
    m_text.setFillColor(sf::Color::White);
    setText(utf8Text);
}

void Label::setText(const std::string& utf8Text) {
    if (utf8Text.empty()) {
        m_text.setString("");
    } else {
        m_text.setString(sf::String::fromUtf8(utf8Text.begin(), utf8Text.end()));
    }
}

void Label::setPosition(float x, float y) {
    m_text.setPosition({x, y});
}

void Label::setPosition(sf::Vector2f pos) {
    m_text.setPosition(pos);
}

void Label::setCenteredX(float centerX, float y) {
    float width = m_text.getGlobalBounds().size.x;
    m_text.setPosition({centerX - width / 2.f, y});
}

void Label::setColor(sf::Color color) {
    m_text.setFillColor(color);
}

void Label::setFontSize(unsigned int size) {
    m_text.setCharacterSize(size);
}

sf::FloatRect Label::getBounds() const {
    return m_text.getGlobalBounds();
}

sf::Vector2f Label::getSize() const {
    auto bounds = m_text.getGlobalBounds();
    return {bounds.size.x, bounds.size.y};
}

void Label::render(sf::RenderWindow& window) const {
    window.draw(m_text);
}