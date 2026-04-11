#include "Button.hpp"
#include "Colors.hpp"

Button::Button(sf::Vector2f pos, sf::Vector2f size, const std::string& label)
    : m_bounds(pos, size), m_label(label) {}

void Button::render(sf::RenderWindow& window, const sf::Font& font, sf::Vector2f mousePos) const {
    // 1. Рисуем фон кнопки
    sf::RectangleShape shape(m_bounds.size);
    shape.setPosition(m_bounds.position);

    bool isHovered = m_enabled && m_bounds.contains(mousePos);

    if (!m_enabled) {
        shape.setFillColor(sf::Color(35, 35, 35, 140)); // Тусклый для выключенной
    } else if (isHovered) {
        shape.setFillColor(sf::Color(72, 72, 72, 230)); // Светлее при наведении
    } else {
        shape.setFillColor(Colors::slotBg); // Стандартный из Colors.hpp
    }
    
    window.draw(shape);

    // 2. Рисуем текст (Создаем локально, как в ревью)
    if (!m_label.empty()) {
        sf::Text text(font, sf::String::fromUtf8(m_label.begin(), m_label.end()), m_fontSize);
        
        if (m_enabled) {
            text.setFillColor(sf::Color::White);
        } else {
            text.setFillColor(sf::Color(80, 80, 80));
        }

        // Центрируем текст внутри кнопки
        sf::FloatRect textBounds = text.getGlobalBounds();
        text.setPosition({
            m_bounds.position.x + (m_bounds.size.x - textBounds.size.x) / 2.f,
            m_bounds.position.y + (m_bounds.size.y - textBounds.size.y) / 2.f - 4.f // -4 для оптической компенсации
        });

        window.draw(text);
    }
}

bool Button::contains(sf::Vector2f pos) const {
    return m_enabled && m_bounds.contains(pos);
}

void Button::setEnabled(bool value) { m_enabled = value; }
bool Button::isEnabled() const { return m_enabled; }

void Button::setLabel(const std::string& newLabel) { m_label = newLabel; }
void Button::setFontSize(unsigned int size) { m_fontSize = size; }

sf::FloatRect Button::getBounds() const { return m_bounds; }

void Button::setPosition(sf::Vector2f pos) {
    m_bounds.position = pos;
}