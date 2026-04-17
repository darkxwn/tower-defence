#include "ui/Button.hpp"
#include "Colors.hpp"
#include <algorithm>

namespace UI {

// Конструктор текстовой кнопки
Button::Button(const sf::Font& font, const std::string& label, sf::Vector2f size, bool useHover)
    : type(ContentType::TextOnly), useHover(useHover) {
    this->size = size;
    shape.setSize(size);
    shape.setFillColor(sf::Color::Transparent);
    text = std::make_unique<sf::Text>(font);
    text->setString(sf::String::fromUtf8(label.begin(), label.end()));
    text->setCharacterSize(24);
    updateLayout();
}

// Конструктор кнопки-иконки
Button::Button(const sf::Texture& texture, sf::Vector2f size, bool useHover)
    : type(ContentType::ImageOnly), useHover(useHover) {
    this->size = size;
    shape.setSize(size);
    shape.setFillColor(sf::Color::Transparent);
    sprite = std::make_unique<sf::Sprite>(texture);
    updateLayout();
}

// Конструктор комбинированной кнопки
Button::Button(const sf::Texture& texture, const sf::Font& font, const std::string& label, sf::Vector2f size, bool useHover)
    : type(ContentType::ImageAndText), useHover(useHover) {
    this->size = size;
    shape.setSize(size);
    shape.setFillColor(sf::Color::Transparent);
    sprite = std::make_unique<sf::Sprite>(texture);
    text = std::make_unique<sf::Text>(font);
    text->setString(sf::String::fromUtf8(label.begin(), label.end()));
    text->setCharacterSize(18);
    updateLayout();
}

Button::Button(Button&&) noexcept = default;
Button& Button::operator=(Button&&) noexcept = default;
Button::~Button() = default;

// Изменение масштаба иконки
void Button::setIconScale(sf::Vector2f scale) {
    if (sprite) {
        hasCustomScale = true;
        sprite->setScale(scale);
        updateLayout();
    }
}

// Вычисление позиций элементов внутри кнопки
void Button::updateLayout() {
    sf::Vector2f center(size.x / 2.f, size.y / 2.f);

    if (type == ContentType::TextOnly && text) {
        sf::FloatRect bounds = text->getLocalBounds();
        text->setOrigin({ bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f });
        text->setPosition(center);
    }
    else if (type == ContentType::ImageOnly && sprite) {
        sf::FloatRect bounds = sprite->getLocalBounds();
        if (!hasCustomScale && bounds.size.x > 0.f && bounds.size.y > 0.f) {
            float scaleX = size.x / bounds.size.x;
            float scaleY = size.y / bounds.size.y;
            float scale = std::min(scaleX, scaleY);
            sprite->setScale({ scale, scale });
            bounds = sprite->getLocalBounds();
        }
        sprite->setOrigin({ bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f });
        sprite->setPosition(center);
    }
    else if (type == ContentType::ImageAndText && sprite && text) {
        sf::FloatRect sBounds = sprite->getLocalBounds();
        sprite->setOrigin({ sBounds.position.x + sBounds.size.x / 2.f, sBounds.position.y + sBounds.size.y / 2.f });
        sprite->setPosition({ center.x, size.y * 0.40f });

        sf::FloatRect tBounds = text->getLocalBounds();
        text->setOrigin({ tBounds.position.x + tBounds.size.x / 2.f, tBounds.position.y + tBounds.size.y / 2.f });
        text->setPosition({ center.x, size.y * 0.875f });
    }
}

// Обработка событий ввода
void Button::handleEvent(const sf::Event& event, const sf::RenderWindow& window, const sf::View& uiView) {
    if (!enabled || !visible) return;

    sf::Vector2i pixelPos;
    bool interactionOccurred = false;
    bool isRelease = false;

    // извлечение координат из разных типов событий SFML 3
    if (const auto* mouseMove = event.getIf<sf::Event::MouseMoved>()) {
        pixelPos = mouseMove->position;
        interactionOccurred = true;
    }
    else if (const auto* touchMove = event.getIf<sf::Event::TouchMoved>()) {
        pixelPos = touchMove->position;
        interactionOccurred = true;
    }
    else if (const auto* mouseRelease = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (mouseRelease->button == sf::Mouse::Button::Left) {
            pixelPos = mouseRelease->position;
            interactionOccurred = true;
            isRelease = true;
        }
    }
    else if (const auto* touchEnd = event.getIf<sf::Event::TouchEnded>()) {
        pixelPos = touchEnd->position;
        interactionOccurred = true;
        isRelease = true;
    }

    if (!interactionOccurred) return;

    // перевод пикселей в координаты вида (учитывает вьюпорт и скролл)
    sf::Vector2f mousePos = window.mapPixelToCoords(pixelPos, uiView);
    isHovered = shape.getGlobalBounds().contains(mousePos);

    // срабатывание только при отпускании кнопки над виджетом
    if (isRelease && isHovered && callback) {
        callback();
    }
}

// Отрисовка кнопки и отладочных элементов
void Button::render(sf::RenderWindow& window) const {
    if (!visible) return;

    // отрисовка фона
    if (!transparent) {
        sf::RectangleShape drawShape = shape;
        if (!enabled) drawShape.setFillColor(Colors::Theme::WidgetDisabled);
        else if (isHovered && useHover) drawShape.setFillColor(Colors::Theme::WidgetHover);
        else drawShape.setFillColor(Colors::Theme::WidgetBg);
        window.draw(drawShape);
    }

    // отрисовка контента
    sf::Vector2f pos = position;
    if (sprite) {
        sf::Sprite s = *sprite;
        s.setPosition(pos + sprite->getPosition());
        window.draw(s);
    }
    if (text) {
        sf::Text t = *text;
        t.setPosition(pos + text->getPosition());
        window.draw(t);
    }

    // отрисовка отладочной рамки (ярко-зеленая для кнопок)
    if (drawOutline) {
        sf::RectangleShape debugRect = shape;
        debugRect.setFillColor(sf::Color::Transparent);
        debugRect.setOutlineColor(sf::Color::Green);
        debugRect.setOutlineThickness(1.f);
        window.draw(debugRect);
    }
}

// Получение границ
sf::FloatRect Button::getGlobalBounds() const {
    return shape.getGlobalBounds();
}

// Изменение обратного вызова
void Button::setCallback(std::function<void()> callback) {
    this->callback = std::move(callback);
}

// Изменение активности
void Button::setEnabled(bool enabled) {
    this->enabled = enabled;
    if (!enabled) isHovered = false;
}

// Изменение текстуры
void Button::setTexture(const sf::Texture& texture) {
    if (!sprite) sprite = std::make_unique<sf::Sprite>(texture);
    else sprite->setTexture(texture);
    updateLayout();
}

// Изменение текста
void Button::setText(const std::string& label) {
    if (text) {
        text->setString(sf::String::fromUtf8(label.begin(), label.end()));
        updateLayout();
    }
}

// Изменение прозрачности
void Button::setTransparent(bool value) {
    transparent = value;
}

// Изменение позиции
void Button::setPosition(sf::Vector2f pos) {
    position = pos;
    shape.setPosition(pos);
}

// Изменение цвета текста
void Button::setTextColor(sf::Color color) {
    if (text) text->setFillColor(color);
}

// Изменение размера текста
void Button::setTextSize(unsigned int size) {
    if (text) {
        text->setCharacterSize(size);
        updateLayout();
    }
}

// Изменение видимости отладочной рамки
void Button::setDrawOutline(bool draw) {
    drawOutline = draw;
}

}
