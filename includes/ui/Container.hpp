#pragma once
#include "ui/Widget.hpp"
#include "ui/NineSlice.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС CONTAINER
//
///////////////////////////////////////////////////////////////////////////

namespace UI {
    class Container : public Widget {
    public:
        // Направление главной оси (расположение элементов)
        enum class Direction { Row, Column };

        // Выравнивание по главной оси (content-alignment)
        enum class ContentAlign { Start, Center, End };

        // Выравнивание по поперечной оси (items-alignment)
        enum class ItemAlign { Start, Center, End };

    private:
        Direction direction = Direction::Column; // текущее направление оси
        ContentAlign contentAlign = ContentAlign::Start; // выравнивание по главной оси
        ItemAlign itemAlign = ItemAlign::Start; // выравнивание по поперечной оси
        bool wrap = false; // флаг переноса элементов на новую строку
        float gap = 0.f; // расстояние между элементами
        sf::Vector2f padding; // внутренние отступы контейнера

        sf::RectangleShape background; // фигура фона
        bool drawBackground = false; // флаг отрисовки фона
        sf::Color bgColor = sf::Color(30, 30, 30, 200); // цвет заливки фона

        sf::RectangleShape outline; // фигура отладочной рамки
        bool drawOutline = false; // флаг отрисовки рамки

        std::unique_ptr<NineSlice> backgroundSlice; // объект для отрисовки растягиваемого фона
        std::vector<std::unique_ptr<Widget>> children; // список дочерних виджетов

        float scrollOffset = 0.f; // текущее смещение прокрутки по вертикали
        bool scrollEnabled = false; // флаг активности прокрутки
        float maxContentHeight = 0.f; // общая высота всего содержимого
        sf::Vector2i lastTouchPos; // последняя позиция касания для скролла
        bool isDragging = false; // флаг процесса перетаскивания (скролла)

        // Расчет позиций и размеров дочерних элементов
        void recalculateLayout();

    public:
        // Инициализация контейнера начальным размером
        Container(sf::Vector2f size);

        // Изменение позиции контейнера
        void setPosition(sf::Vector2f pos) override;

        // Изменение размера контейнера
        void setSize(sf::Vector2f size) override;

        // Изменение направления главной оси
        void setDirection(Direction dir);

        // Изменение выравнивания по главной оси
        void setContentAlign(ContentAlign align);

        // Изменение выравнивания по поперечной оси
        void setItemAlign(ItemAlign align);

        // Изменение состояния переноса элементов
        void setWrap(bool value);

        // Изменение отступа между элементами
        void setGap(float value);

        // Изменение внутренних отступов
        void setPadding(sf::Vector2f value);

        // Изменение цвета фона
        void setBackgroundColor(sf::Color color);

        // Изменение видимости фона
        void setDrawBackground(bool draw);

        // Изменение текстуры фона с индивидуальными отступами
        void setBackgroundTexture(const sf::Texture& tex, float left, float top, float right, float bottom);

        // Изменение текстуры фона с одинаковым отступом для всех сторон
        void setBackgroundTexture(const sf::Texture& tex, float edge);

        // Изменение видимости отладочной рамки
        void setDrawOutline(bool draw);

        // Изменение активности прокрутки
        void setScrollEnabled(bool enabled);

        // Проверка, находится ли контейнер в процессе перетаскивания
        bool isCurrentlyDragging() const;

        // Добавление виджета в контейнер
        void addChild(std::unique_ptr<Widget> child);

        // Удаление виджета по индексу
        void removeChild(size_t index);

        // Удаление всех виджетов из контейнера
        void clearChildren();

        // Получение указателя на виджет по индексу
        Widget* getChild(size_t index) const;

        // Получение общего количества виджетов
        size_t getChildrenCount() const;

        // Принудительный пересчет позиций всех элементов
        void rebuild();

        // Обработка системных событий
        void handleEvent(const sf::Event& event, const sf::RenderWindow& window, const sf::View& uiView) override;

        // Отрисовка контейнера и вложенных элементов
        void render(sf::RenderWindow& window) const override;

        // Получение глобальных границ
        sf::FloatRect getGlobalBounds() const override;
    };
}
