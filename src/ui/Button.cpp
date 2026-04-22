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
Button::Button(const sf::Texture& texture, const sf::Font& font, const std::string& label, sf::Vector2f size, IconPlacement placement, bool useHover)
    : type(ContentType::ImageAndText), placement(placement), useHover(useHover) {
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

// Изменение расположения иконки
void Button::setIconPlacement(IconPlacement placement) {
    this->placement = placement;
    updateLayout();
}

// Изменение отступа между контентом
void Button::setContentGap(float gap) {
    this->contentGap = gap;
    updateLayout();
}

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
    float padding = size.x * 0.05f; // отступ от краев для иконок 

    if (type == ContentType::TextOnly && text) {
        sf::FloatRect bounds = text->getLocalBounds();
        text->setOrigin({ bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f });
        text->setPosition(center);
    }
    else if (type == ContentType::ImageOnly && sprite) {
        sf::FloatRect bounds = sprite->getLocalBounds();
        if (!hasCustomScale && bounds.size.x > 0.f && bounds.size.y > 0.f) {
            float scaleX = (size.x * 0.8f) / bounds.size.x;
            float scaleY = (size.y * 0.8f) / bounds.size.y;
            float scale = std::min(scaleX, scaleY);
            sprite->setScale({ scale, scale });
            bounds = sprite->getGlobalBounds();
        }
        sf::FloatRect sLocal = sprite->getLocalBounds();
        sprite->setOrigin({ sLocal.position.x + sLocal.size.x / 2.f, sLocal.position.y + sLocal.size.y / 2.f });
        sprite->setPosition(center);
    }
    else if (type == ContentType::ImageAndText && sprite && text) {
        sf::FloatRect sBounds = sprite->getGlobalBounds();
        sf::FloatRect sLocal = sprite->getLocalBounds();
        sprite->setOrigin({ sLocal.position.x + sLocal.size.x / 2.f, sLocal.position.y + sLocal.size.y / 2.f });

        sf::FloatRect tBounds = text->getLocalBounds();
        text->setOrigin({ tBounds.position.x + tBounds.size.x / 2.f, tBounds.position.y + tBounds.size.y / 2.f });

        if (placement == IconPlacement::Top) {
            // Центрируем как единый блок (кластер)
            float totalH = sBounds.size.y + contentGap + tBounds.size.y;
            float startY = (size.y - totalH) / 2.f;
            sprite->setPosition({ center.x, startY + sBounds.size.y / 2.f + 2.5f });
            text->setPosition({ center.x, startY + sBounds.size.y + contentGap + tBounds.size.y / 2.f - 2.5f });
        }
        else if (placement == IconPlacement::Left) {
            // Иконка слева у края, текст в оставшейся части
            sprite->setPosition({ padding + sBounds.size.x / 2.f, center.y });
            
            float remainingSpaceX = size.x - (padding + sBounds.size.x + contentGap);
            text->setPosition({ (padding + sBounds.size.x + contentGap) + remainingSpaceX / 2.f, center.y });
        }
        else if (placement == IconPlacement::Right) {
            // Текст слева, иконка справа у края
            sprite->setPosition({ size.x - padding - sBounds.size.x / 2.f, center.y });

            float remainingSpaceX = size.x - (padding + sBounds.size.x + contentGap);
            text->setPosition({ remainingSpaceX / 2.f, center.y });
        }
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

    // отрисовка контента с учетом позиции кнопки
    if (sprite) {
        sf::Vector2f originalPos = sprite->getPosition();
        sprite->setPosition(position + originalPos);
        window.draw(*sprite);
        sprite->setPosition(originalPos); // восстанавливаем локальную позицию
    }
    if (text) {
        sf::Vector2f originalPos = text->getPosition();
        text->setPosition(position + originalPos);
        window.draw(*text);
        text->setPosition(originalPos);
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

// Изменение размеров
void Button::setSize(sf::Vector2f size) {
    this->size = size;
    shape.setSize(size);
    updateLayout();
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

// Изменение цвета иконки (цветовой фильтр)
void Button::setIconColor(sf::Color color) {
    if (sprite) {
        sprite->setColor(color);
    }
}

// Изменение видимости отладочной рамки
void Button::setDrawOutline(bool draw) {
    drawOutline = draw;
}

}
