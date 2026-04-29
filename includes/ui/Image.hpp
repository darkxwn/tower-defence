#pragma once
#include "ui/Widget.hpp"
#include <SFML/Graphics.hpp>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС IMAGE
// Простой виджет для отображения графических ресурсов (иконок, картинок).
// Поддерживает автоматическое масштабирование под заданный размер.
//
///////////////////////////////////////////////////////////////////////////

namespace UI {
    class Image : public Widget {
    private:
        sf::Sprite sprite;              // Спрайт для отрисовки
        sf::Vector2f customScale = { 1.f, 1.f }; // Дополнительный множитель масштаба

        // Обновление масштаба спрайта на основе размера виджета и customScale
        void updateScale();

    public:
        // Конструктор: принимает текстуру, размер устанавливается по размеру текстуры
        Image(const sf::Texture& texture);
        
        // Конструктор: принимает текстуру и желаемый размер виджета
        Image(const sf::Texture& texture, sf::Vector2f size);

        // Обработка событий (не требуется)
        void handleEvent(const sf::Event& event, const sf::RenderWindow& window, const sf::View& uiView) override;

        // Отрисовка изображения в окно
        void render(sf::RenderWindow& window) const override;

        // Возвращает глобальные границы спрайта
        sf::FloatRect getGlobalBounds() const override;

        // Установка позиции виджета и спрайта
        void setPosition(sf::Vector2f pos) override;

        // Установка размера виджета
        void setSize(sf::Vector2f size) override;

        // Установка новой текстуры
        void setTexture(const sf::Texture& texture);

        // Установка множителя масштаба 
        void setScale(sf::Vector2f scale);

        // Установка цвета или прозрачности (фильтр)
        void setColor(sf::Color color);
    };
}
