Вот полный технический план по внедрению системы **9-Slice Scaling** в ваш проект. Все изменения и новые файлы собраны в один документ.

# 🛠 ПЛАН ВНЕДРЕНИЯ СИСТЕМЫ 9-SLICE UI (V4.0)

## 1. СОЗДАНИЕ КЛАССА NINESLICE

Этот класс является вспомогательным инструментом для отрисовки растягиваемых текстур с сохранением пропорций углов.

### 📄 Файл: `includes/ui/NineSlice.hpp`
```cpp
#pragma once
#include <SFML/Graphics.hpp>

namespace UI {
    class NineSlice : public sf::Drawable, public sf::Transformable {
    private:
        const sf::Texture* texture = nullptr;
        sf::VertexArray vertices;
        sf::Vector2f size;
        float left = 0.f, top = 0.f, right = 0.f, bottom = 0.f;
        sf::Color color = sf::Color::White;

        void updateVertices();

    public:
        NineSlice();
        
        // Основная инициализация
        void setTexture(const sf::Texture& tex, float l, float t, float r, float b);
        
        // Быстрая смена текстуры без пересчета геометрии (для состояний)
        void swapTexture(const sf::Texture* newTex); 
        
        void setSize(sf::Vector2f newSize);
        void setColor(sf::Color newColor);
        sf::Vector2f getSize() const { return size; }
        bool hasTexture() const { return texture != nullptr; }

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    };
}
```

### 📄 Файл: `src/ui/NineSlice.cpp`
```cpp
#include "ui/NineSlice.hpp"

namespace UI {
    NineSlice::NineSlice() : vertices(sf::PrimitiveType::Triangles, 54) {}

    void NineSlice::setTexture(const sf::Texture& tex, float l, float t, float r, float b) {
        texture = &tex;
        left = l; top = t; right = r; bottom = b;
        updateVertices();
    }

    void NineSlice::swapTexture(const sf::Texture* newTex) {
        if (texture != newTex && newTex) texture = newTex;
    }

    void NineSlice::setSize(sf::Vector2f newSize) {
        if (size != newSize) { size = newSize; updateVertices(); }
    }

    void NineSlice::setColor(sf::Color newColor) {
        color = newColor;
        for (std::size_t i = 0; i < vertices.getVertexCount(); ++i) vertices[i].color = color;
    }

    void NineSlice::updateVertices() {
        if (!texture || size.x <= 0 || size.y <= 0) return;
        sf::Vector2f ts(texture->getSize());
        
        float xC[4] = { 0.f, left, size.x - right, size.x };
        float yC[4] = { 0.f, top, size.y - bottom, size.y };
        float uC[4] = { 0.f, left, ts.x - right, ts.x };
        float vC[4] = { 0.f, top, ts.y - bottom, ts.y };

        int vi = 0;
        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                sf::Vector2f p0(xC[x], yC[y]), p1(xC[x+1], yC[y]), p2(xC[x], yC[y+1]), p3(xC[x+1], yC[y+1]);
                sf::Vector2f uv0(uC[x], vC[y]), uv1(uC[x+1], vC[y]), uv2(uC[x], vC[y+1]), uv3(uC[x+1], vC[y+1]);
                
                vertices[vi++] = sf::Vertex(p0, color, uv0);
                vertices[vi++] = sf::Vertex(p1, color, uv1);
                vertices[vi++] = sf::Vertex(p2, color, uv2);
                vertices[vi++] = sf::Vertex(p2, color, uv2);
                vertices[vi++] = sf::Vertex(p1, color, uv1);
                vertices[vi++] = sf::Vertex(p3, color, uv3);
            }
        }
    }

    void NineSlice::draw(sf::RenderTarget& target, sf::RenderStates states) const {
        if (texture) { 
            states.transform *= getTransform(); 
            states.texture = texture; 
            target.draw(vertices, states); 
        }
    }
}
```

---

## 2. ИНТЕГРАЦИЯ В КЛАСС BUTTON

### 📄 Файл: `includes/ui/Button.hpp`
**Добавьте следующие поля в `private`:**
```cpp
UI::NineSlice backgroundSlice;
bool useNineSlice = false;
const sf::Texture *texNormal = nullptr, *texHover = nullptr, *texDisabled = nullptr;

// Обновление текстуры в зависимости от состояния
void updateVisualState(); 
```

**Добавьте следующие методы в `public`:**
```cpp
// Установка текстур для состояний (edges - толщина нетянущихся краев)
void setBackgroundTextures(const sf::Texture* normal, const sf::Texture* hover, const sf::Texture* disabled, float edge);
void setBackgroundTextures(const sf::Texture* normal, const sf::Texture* hover, const sf::Texture* disabled, float l, float t, float r, float b);
```

### 📄 Файл: `src/ui/Button.cpp`

**1. Реализация инициализации текстур:**
```cpp
void Button::setBackgroundTextures(const sf::Texture* n, const sf::Texture* h, const sf::Texture* d, float l, float t, float r, float b) {
    useNineSlice = true; 
    texNormal = n; texHover = h; texDisabled = d;
    if (n) { 
        backgroundSlice.setTexture(*n, l, t, r, b); 
        backgroundSlice.setSize(size); 
        backgroundSlice.setPosition(position);
    }
    updateVisualState();
}

void Button::setBackgroundTextures(const sf::Texture* n, const sf::Texture* h, const sf::Texture* d, float edge) {
    setBackgroundTextures(n, h, d, edge, edge, edge, edge);
}
```

**2. Реализация переключения состояний:**
```cpp
void Button::updateVisualState() {
    if (!useNineSlice) return;

    if (!enabled && texDisabled) {
        backgroundSlice.swapTexture(texDisabled);
    } else if (isHovered && useHover && texHover) {
        backgroundSlice.swapTexture(texHover);
    } else if (texNormal) {
        backgroundSlice.swapTexture(texNormal);
    }
}
```

**3. Модификация `handleEvent`:**
Найдите место, где обновляется `isHovered`. Замените логику на проверку изменения состояния:
```cpp
bool prevHover = isHovered;
isHovered = shape.getGlobalBounds().contains(mousePos);

// Если состояние наведения изменилось — обновляем визуал
if (prevHover != isHovered) {
    updateVisualState();
}
```

**4. Модификация `setEnabled`, `setPosition`, `setSize`:**
В каждый из этих методов добавьте вызов обновления 9-slice:
*   В `setEnabled`: добавьте `updateVisualState();` в конец.
*   В `setPosition`: добавьте `backgroundSlice.setPosition(pos);`.
*   В `setSize`: добавьте `backgroundSlice.setSize(size);`.

**5. Модификация `render`:**
Замените блок отрисовки фона на выбор между цветом и текстурой:
```cpp
if (!transparent) {
    if (useNineSlice) {
        window.draw(backgroundSlice);
    } else {
        sf::RectangleShape drawShape = shape;
        if (!enabled) drawShape.setFillColor(Colors::Theme::WidgetDisabled);
        else if (isHovered && useHover) drawShape.setFillColor(Colors::Theme::WidgetHover);
        else drawShape.setFillColor(Colors::Theme::Widget);
        window.draw(drawShape);
    }
}
```

---

## 3. ИНТЕГРАЦИЯ В КЛАСС CONTAINER

### 📄 Файл: `includes/ui/Container.hpp`
**Добавьте в `private`:**
```cpp
UI::NineSlice nineSliceBg;
bool useNineSlice = false;
```

**Добавьте в `public`:**
```cpp
void setBackgroundTexture(const sf::Texture& tex, float edge);
```

### 📄 Файл: `src/ui/Container.cpp`

**1. Реализация установки текстуры:**
```cpp
void Container::setBackgroundTexture(const sf::Texture& tex, float edge) {
    useNineSlice = true;
    nineSliceBg.setTexture(tex, edge, edge, edge, edge);
    nineSliceBg.setSize(size);
    nineSliceBg.setPosition(position);
}
```

**2. Обновление `setPosition` и `setSize`:**
В конец этих методов добавьте:
```cpp
if (useNineSlice) {
    nineSliceBg.setPosition(position);
    nineSliceBg.setSize(size);
}
```

**3. Модификация `render`:**
Замените отрисовку фона:
```cpp
if (drawBackground) {
    if (useNineSlice) window.draw(nineSliceBg);
    else window.draw(background);
}
```

---

## 4. ПРИМЕР ИСПОЛЬЗОВАНИЯ В КОДЕ МЕНЮ

После внедрения изменений вы можете использовать новые методы в `Menu.cpp`:

```cpp
// Настройка кнопки с 3 текстурами и рамкой в 15 пикселей
auto playBtn = std::make_unique<UI::Button>(font, "ИГРАТЬ", sf::Vector2f(200, 60));
playBtn->setBackgroundTextures(
    &ResourceManager::get("btn_normal"), 
    &ResourceManager::get("btn_hover"), 
    &ResourceManager::get("btn_disabled"), 
    15.0f
);

// Настройка панели меню (статичный 9-slice)
mainContainer->setDrawBackground(true);
mainContainer->setBackgroundTexture(ResourceManager::get("panel_bg"), 24.0f);
```