#include "ui/NineSlice.hpp"

namespace UI {
    // Инициализация с текстурой и одинаковыми отступами
    NineSlice::NineSlice(const sf::Texture& tex, float edge) : vertices(sf::PrimitiveType::Triangles, 54) {
        setTexture(tex, edge);
    }

    // Инициализация с текстурой и индивидуальными отступами
    NineSlice::NineSlice(const sf::Texture& tex, float l, float t, float r, float b) : vertices(sf::PrimitiveType::Triangles, 54) {
        setTexture(tex, l, t, r, b);
    }

    // Изменение параметров текстуры и сетки
    void NineSlice::setTexture(const sf::Texture& tex, float l, float t, float r, float b) {
        texture = &tex;
        left = l; 
        top = t; 
        right = r; 
        bottom = b;
        updateVertices();
    }

    // Изменение текстуры с одинаковыми отступами
    void NineSlice::setTexture(const sf::Texture& tex, float edge) {
        setTexture(tex, edge, edge, edge, edge);
    }

    // Изменение указателя на текстуру без пересчета вершин
    void NineSlice::swapTexture(const sf::Texture* newTex) {
        if (texture != newTex && newTex) {
            texture = newTex;
        }
    }

    // Изменение размера с последующим обновлением геометрии
    void NineSlice::setSize(sf::Vector2f newSize) {
        if (size != newSize) { 
            size = newSize; 
            updateVertices(); 
        }
    }

    // Изменение цвета всех вершин
    void NineSlice::setColor(sf::Color newColor) {
        color = newColor;
        for (std::size_t i = 0; i < vertices.getVertexCount(); ++i) {
            vertices[i].color = color;
        }
    }

    // Получение текущего размера
    sf::Vector2f NineSlice::getSize() const {
        return size;
    }

    // Проверка наличия текстуры
    bool NineSlice::hasTexture() const {
        return texture != nullptr;
    }

    // Расчет позиций и текстурных координат для сегментов сетки
    void NineSlice::updateVertices() {
        if (!texture || size.x <= 0.f || size.y <= 0.f) {
            return;
        }

        sf::Vector2f ts = static_cast<sf::Vector2f>(texture->getSize());
        
        // экранные координаты сетки
        float xC[4] = { 0.f, left, size.x - right, size.x };
        float yC[4] = { 0.f, top, size.y - bottom, size.y };
        
        // базовые текстурные координаты
        float uC[4] = { 0.f, left, ts.x - right, ts.x };
        float vC[4] = { 0.f, top, ts.y - bottom, ts.y };

        // отступ для предотвращения швов (texture bleeding)
        float eps = 0.45f; 

        int vi = 0;
        // формирование треугольников для каждого из девяти секторов
        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                // экранные позиции углов сектора
                sf::Vector2f p0(xC[x], yC[y]), p1(xC[x + 1], yC[y]), p2(xC[x], yC[y + 1]), p3(xC[x + 1], yC[y + 1]);
                
                // текстурные координаты с коррекцией растягиваемых зон
                float u0 = uC[x], u1 = uC[x + 1];
                float v0 = vC[y], v1 = vC[y + 1];

                // сжатие UV для центральных (растягиваемых) сегментов по горизонтали
                if (x == 1) { 
                    u0 += eps; 
                    u1 -= eps; 
                }
                // сжатие UV для центральных сегментов по вертикали
                if (y == 1) { 
                    v0 += eps; 
                    v1 -= eps; 
                }

                sf::Vector2f uv0(u0, v0), uv1(u1, v0), uv2(u0, v1), uv3(u1, v1);
                
                // первый треугольник сектора
                vertices[vi++] = sf::Vertex(p0, color, uv0);
                vertices[vi++] = sf::Vertex(p1, color, uv1);
                vertices[vi++] = sf::Vertex(p2, color, uv2);
                
                // второй треугольник сектора
                vertices[vi++] = sf::Vertex(p2, color, uv2);
                vertices[vi++] = sf::Vertex(p1, color, uv1);
                vertices[vi++] = sf::Vertex(p3, color, uv3);
            }
        }
    }

    // Отрисовка с учетом трансформации и текстуры
    void NineSlice::draw(sf::RenderTarget& target, sf::RenderStates states) const {
        if (texture) { 
            states.transform *= getTransform(); 
            states.texture = texture; 
            target.draw(vertices, states); 
        }
    }
}
