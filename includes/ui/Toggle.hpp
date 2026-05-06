#pragma once
#include "ui/Widget.hpp"
#include <SFML/Graphics.hpp>
#include <functional>
#include "Colors.hpp"

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС TOGGLE
//
///////////////////////////////////////////////////////////////////////////

namespace UI {
    class Toggle : public Widget {
    private:
        bool isToggled = false; // текущее состояние

        sf::CircleShape trackLeft;   // левый край трека
        sf::CircleShape trackRight;  // правый край трека
        sf::RectangleShape trackBody; // тело трека
        
        sf::CircleShape handle; // круглая ручка (тумблер)

        std::function<void(bool)> onToggle; // обратный вызов при переключении

        //// Цвета
        //sf::Color trackColorOff = Colors::Theme::WidgetDisabled;
        //sf::Color trackColorOn  = Colors::Theme::Widget; 
        //sf::Color handleColor   = Colors::Palette::White;

        bool isHovered = false; // состояние наведения

        // Обновление геометрии и цветов на основе состояния
        void updateVisuals();

    public:
        // Конструктор
        Toggle(bool initialState, sf::Vector2f size);

        // Обработка событий ввода
        void handleEvent(const sf::Event& event, const sf::RenderWindow& window, const sf::View& uiView) override;

        // Отрисовка
        void render(sf::RenderWindow& window) const override;

        // Получение глобальных границ
        sf::FloatRect getGlobalBounds() const override;

        // Изменение позиции
        void setPosition(sf::Vector2f pos) override;

        // Установка коллбека
        void setCallback(std::function<void(bool)> callback);

        // Изменение состояния
        void setToggled(bool value);

        // Получение состояния
        bool getToggled() const;
    };
}
