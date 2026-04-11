#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class Label {
private:
    sf::Text m_text;

public:
    Label(const sf::Font& font, const std::string& utf8Text = "", unsigned int fontSize = 24);

    void setText(const std::string& utf8Text);
    
    void setPosition(float x, float y);
    void setPosition(sf::Vector2f pos);
    void setCenteredX(float centerX, float y);

    // Настройка внешнего вида
    void setColor(sf::Color color);
    void setFontSize(unsigned int size);

    // Геттеры для расчетов
    sf::FloatRect getBounds() const;
    sf::Vector2f getSize() const;

    void render(sf::RenderWindow& window) const;
};