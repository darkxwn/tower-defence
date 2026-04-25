#pragma once
#include "ui/Widget.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <functional>
#include <memory>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС BUTTON
//
///////////////////////////////////////////////////////////////////////////

namespace UI {
    // Варианты расположения иконки относительно текста
    enum class IconPlacement {
        Left, // иконка слева, текст справа
        Right, // текст слева, иконка справа
        Top // иконка сверху, текст снизу
    };

    class Button : public Widget {
    public:
        enum class ContentType { TextOnly, ImageOnly, ImageAndText };

    private:
        ContentType type; // тип содержимого кнопки
        IconPlacement placement = IconPlacement::Left; // расположение иконки
        sf::RectangleShape shape; // фоновая фигура
        std::unique_ptr<sf::Sprite> sprite; // иконка
        std::unique_ptr<sf::Text> text; // текст

        bool useHover; // эффект наведения
        bool isHovered = false; // состояние наведения
        bool transparent = false; // прозрачный фон
        bool hasCustomScale = false; // ручной масштаб иконки
        bool drawOutline = false; // отрисовка отладочной рамки
        float contentGap = 5.f; // отступ между иконкой и текстом

        std::function<void()> callback; // обратный вызов

        // Вычисляет внутреннее положение текста и иконки
        void updateLayout();

    public:
        // Конструктор пустой кнопки
        Button() : type(ContentType::TextOnly), useHover(false) {}

        // Конструктор текстовой кнопки
        Button(const sf::Font& font, const std::string& label, sf::Vector2f size, bool useHover = true);

        // Конструктор кнопки-иконки
        Button(const sf::Texture& texture, sf::Vector2f size, bool useHover = true);

        // Конструктор комбинированной кнопки
        Button(const sf::Texture& texture, const sf::Font& font, const std::string& label, sf::Vector2f size, IconPlacement placement = IconPlacement::Left, bool useHover = true);

        // Конструктор перемещения
        Button(Button&&) noexcept;
        Button& operator=(Button&&) noexcept;
        ~Button() override;

        // Обработка событий ввода
        void handleEvent(const sf::Event& event, const sf::RenderWindow& window, const sf::View& uiView) override;

        // Отрисовка кнопки
        void render(sf::RenderWindow& window) const override;

        // Получение глобальных границ
        sf::FloatRect getGlobalBounds() const override;

        // Изменение обратного вызова
        void setCallback(std::function<void()> callback);

        // Изменение позиции кнопки
        void setPosition(sf::Vector2f pos) override;

        // Изменение размера кнопки
        void setSize(sf::Vector2f size) override;

        // Изменение активности кнопки
        void setEnabled(bool enabled) override;

        // Изменение текстуры спрайта
        void setTexture(const sf::Texture& texture);

        // Изменение текста кнопки
        void setText(const std::string& label);

        // Изменение расположения иконки
        void setIconPlacement(IconPlacement placement);

        // Изменение отступа между контентом
        void setContentGap(float gap);

        // Изменение масштаба иконки
        void setIconScale(sf::Vector2f scale);

        // Изменение прозрачности фона
        void setTransparent(bool value);

        // Изменение цвета текста
        void setTextColor(sf::Color color);

        // Изменение размера текста
        void setTextSize(unsigned int size);

        // Получение указателя на текст (для обновления)
        sf::Text* getTextPtr();

        // Изменение цвета иконки (цветовой фильтр)
        void setIconColor(sf::Color color);

        // Изменение видимости отладочной рамки
        void setDrawOutline(bool draw);
    };
}
