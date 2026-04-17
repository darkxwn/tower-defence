#pragma once
#include <SFML/Graphics.hpp>
#include <functional>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС WIDGET
//
///////////////////////////////////////////////////////////////////////////

namespace UI {
    class Widget {
    protected:
        sf::Vector2f position;   // позиция элемента
        sf::Vector2f size;       // размер элемента
        bool enabled = true;     // активность элемента
        bool visible = true;     // видимость элемента
        bool followsLayout = true; // участие в автоматической расстановке

    public:
        virtual ~Widget() = default;

        // Обработка системных событий
        virtual void handleEvent(const sf::Event& event, const sf::RenderWindow& window, const sf::View& uiView) = 0;

        // Отрисовка элемента
        virtual void render(sf::RenderWindow& window) const = 0;

        // Получение глобальных границ
        virtual sf::FloatRect getGlobalBounds() const = 0;

        // Изменение позиции
        virtual void setPosition(sf::Vector2f pos);

        // Получение позиции
        sf::Vector2f getPosition() const;

        // Изменение размера
        virtual void setSize(sf::Vector2f size);

        // Получение размера
        sf::Vector2f getSize() const;

        // Изменение активности
        virtual void setEnabled(bool enabled);

        // Проверка активности
        bool isEnabled() const;

        // Изменение видимости
        void setVisible(bool visible);

        // Проверка видимости
        bool isVisible() const;

        // Изменение статуса участия в расстановке
        void setFollowsLayout(bool value);

        // Получение статуса участия в расстановке
        bool getFollowsLayout() const;
    };
}
