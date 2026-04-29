#pragma once
#include <SFML/Graphics.hpp>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС NINESLICE
//
///////////////////////////////////////////////////////////////////////////

namespace UI {
    // Класс для отрисовки растягиваемой текстуры с сохранением пропорций углов
    class NineSlice : public sf::Drawable, public sf::Transformable {
    private:
        const sf::Texture* texture = nullptr; // указатель на текущую текстуру
        sf::VertexArray vertices; // массив вершин для отрисовки
        sf::Vector2f size; // размер области отрисовки
        float left = 0.f; // отступ слева
        float top = 0.f; // отступ сверху
        float right = 0.f; // отступ справа
        float bottom = 0.f; // отступ снизу
        sf::Color color = sf::Color::White; // цветовой фильтр

        // Перерасчет геометрии вершин
        void updateVertices();

    public:
        // Конструктор с инициализацией текстуры и единого отступа
        NineSlice(const sf::Texture& tex, float edge);

        // Конструктор с инициализацией текстуры и индивидуальных отступов
        NineSlice(const sf::Texture& tex, float left, float top, float right, float bottom);
        
        // Изменение текстуры и параметров сетки
        void setTexture(const sf::Texture& tex, float left, float top, float right, float bottom);

        // Изменение текстуры с единым отступом для всех сторон
        void setTexture(const sf::Texture& tex, float edge);
        
        // Быстрая подмена текстуры без пересчета сетки
        void swapTexture(const sf::Texture* newTex); 
        
        // Изменение размера области отрисовки
        void setSize(sf::Vector2f newSize);

        // Изменение цвета фильтра
        void setColor(sf::Color newColor);

        // Получение текущего размера
        sf::Vector2f getSize() const;

        // Проверка наличия установленной текстуры
        bool hasTexture() const;

    protected:
        // Отрисовка объекта в цель
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    };
}
