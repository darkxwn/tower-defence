#pragma once
#include "ui/Widget.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <memory>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС TEXT
//
///////////////////////////////////////////////////////////////////////////

namespace UI {
    class Text : public Widget {
    public:
        // Режимы выравнивания текста
        enum class Align { Left, Center, Right };

    private:
        std::unique_ptr<sf::Text> text; // объект текста
        std::string rawString; // исходная строка
        float maxWidth = 0.f; // лимит ширины переноса
        Align alignment = Align::Left; // выравнивание текста

        // Перенос текста по ширине
        void applyWrapping();

    public:
        // Конструктор текста
        Text(const sf::Font& font, const std::string& utf8Text = "", unsigned int fontSize = 24);

        // Конструктор текста с размером контейнера для выравнивания
        Text(const sf::Font& font, const std::string& utf8Text, unsigned int fontSize, sf::Vector2f containerSize);

        // Изменение позиции текста
        void setPosition(sf::Vector2f pos) override;

        // Изменение размера (переопределено для пересчёта выравнивания)
        void setSize(sf::Vector2f size) override;

        // Изменение текстового содержимого
        void setText(const std::string& utf8Text);

        // Изменение лимита ширины переноса
        void setMaxWidth(float width);

        // Изменение выравнивания текста
        void setAlignment(Align align);

        // Применение выравнивания к позиции
        void applyAlignment() const;

        // Изменение цвета текста
        void setColor(sf::Color color);

        // Изменение размера шрифта
        void setFontSize(unsigned int fontSize);

        // Получение указателя на внутренний sf::Text
        sf::Text* getText();

        // Изменение межстрочного интервала
        void setLineSpacing(float spacing);

        // Получение локальных границ текста
        sf::FloatRect getLocalBounds() const;

        // Обработка событий ввода
        void handleEvent(const sf::Event& event, const sf::RenderWindow& window, const sf::View& uiView) override;

        // Отрисовка текста
        void render(sf::RenderWindow& window) const override;

        // Получение глобальных границ
        sf::FloatRect getGlobalBounds() const override;
    };
}
