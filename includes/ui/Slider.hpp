#pragma once
#include "ui/Widget.hpp"
#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС SLIDER
//
///////////////////////////////////////////////////////////////////////////

namespace UI {
    class Slider : public Widget {
    private:
        float minValue; // минимальное значение
        float maxValue; // максимальное значение
        float currentValue; // текущее значение

        sf::RectangleShape track; // дорожка слайдера
        sf::RectangleShape handle; // ручка слайдера
        std::unique_ptr<sf::Text> valueText; // текст с текущим значением
        
        sf::Color trackColor; // цвет дорожки
        sf::Color handleColor; // цвет ручки

        bool isDragging = false; // состояние перетаскивания
        std::function<void(float)> onValueChange; // обратный вызов при изменении

        // Обновление положения ручки и текста
        void updateInternalLayout();

    public:
        // Конструктор слайдера
        Slider(const sf::Font& font, float min, float max, float initial, sf::Vector2f size);

        // Обработка событий ввода
        void handleEvent(const sf::Event& event, const sf::RenderWindow& window, const sf::View& uiView) override;

        // Отрисовка слайдера
        void render(sf::RenderWindow& window) const override;

        // Получение глобальных границ
        sf::FloatRect getGlobalBounds() const override;

        // Изменение позиции слайдера
        void setPosition(sf::Vector2f pos) override;

        // Изменение размера слайдера
        void setSize(sf::Vector2f size) override;

        // Получение текущего значения
        float getValue() const;

        // Изменение текущего значения
        void setValue(float value);

        // Изменение обратного вызова
        void setCallback(std::function<void(float)> callback);

        // Изменение цвета дорожки
        void setTrackColor(sf::Color color);

        // Изменение цвета ручки
        void setHandleColor(sf::Color color);
    };
}
