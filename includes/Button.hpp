#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class Button {
private:
    sf::FloatRect m_bounds;
    std::string m_label;
    unsigned int m_fontSize = 24;
    bool m_enabled = true;

public:
    // Конструктор: позиция, размер и текст
    Button(sf::Vector2f pos, sf::Vector2f size, const std::string& label);

    // Отрисовка: передаем окно, шрифт и текущую позицию мыши (для эффекта наведения)
    void render(sf::RenderWindow& window, const sf::Font& font, sf::Vector2f mousePos) const;

    // Проверка попадания
    bool contains(sf::Vector2f pos) const;

    // Управление состоянием
    void setEnabled(bool value);
    bool isEnabled() const;

    // Настройка текста
    void setLabel(const std::string& newLabel);
    void setFontSize(unsigned int size);

    // Геометрия
    sf::FloatRect getBounds() const;
    void setPosition(sf::Vector2f pos);
};