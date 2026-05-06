#include "ui/Toggle.hpp"
#include "Colors.hpp"

namespace UI {

    // Конструктор
    Toggle::Toggle(bool initialState, sf::Vector2f size) : isToggled(initialState) {
        this->size = size;

        updateVisuals();
    }

    // Обновление геометрии и цветов на основе состояния
    void Toggle::updateVisuals() {
        float radius = size.y / 2.f;
        
        // Настройка трека (форма капсулы)
        trackLeft.setRadius(radius);
        trackLeft.setPosition(position);
        
        trackRight.setRadius(radius);
        trackRight.setPosition(position + sf::Vector2f(size.x - size.y, 0.f));
        
        trackBody.setSize(sf::Vector2f(size.x - size.y, size.y));
        trackBody.setPosition(position + sf::Vector2f(radius, 0.f));

        sf::Color currentColor = isToggled ? Colors::Theme::Widget : Colors::Theme::WidgetDisabled;
        if (isHovered && enabled) {
            currentColor = Colors::Theme::WidgetHover;
        }
        
        trackLeft.setFillColor(currentColor);
        trackRight.setFillColor(currentColor);
        trackBody.setFillColor(currentColor);

        // Настройка ручки 
        float handlePadding = size.y * 0.1f; // внутренний отступ ручки
        float handleRadius = radius - handlePadding;
        
        handle.setRadius(handleRadius);
        handle.setFillColor(enabled ? Colors::Palette::White : Colors::Theme::Widget);

        // Позиция ручки зависит от состояния
        float handleY = position.y + handlePadding;
        float handleX = isToggled ? (position.x + size.x - size.y + handlePadding) : (position.x + handlePadding);
        
        handle.setPosition(sf::Vector2f(handleX, handleY));
    }

    // Обработка событий ввода
    void Toggle::handleEvent(const sf::Event& event, const sf::RenderWindow& window, const sf::View& uiView) {
        if (!enabled || !visible) return;

        sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
        sf::Vector2f mousePos = window.mapPixelToCoords(pixelPos, uiView);
        
        bool wasHovered = isHovered;
        isHovered = getGlobalBounds().contains(mousePos);
        
        if (wasHovered != isHovered) {
            updateVisuals();
        }

        if (isHovered) {
            if (const auto* mouseB = event.getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseB->button == sf::Mouse::Button::Left) {
                    isToggled = !isToggled;
                    updateVisuals();
                    if (onToggle) onToggle(isToggled);
                }
            }
            else if (const auto* touchB = event.getIf<sf::Event::TouchBegan>()) {
                isToggled = !isToggled;
                updateVisuals();
                if (onToggle) onToggle(isToggled);
            }
        }
    }

    // Отрисовка
    void Toggle::render(sf::RenderWindow& window) const {
        if (!visible) return;
        window.draw(trackLeft);
        window.draw(trackRight);
        window.draw(trackBody);
        window.draw(handle);
    }

    // Получение глобальных границ
    sf::FloatRect Toggle::getGlobalBounds() const {
        return { position, size };
    }

    // Изменение позиции
    void Toggle::setPosition(sf::Vector2f pos) {
        position = pos;
        updateVisuals();
    }

    // Установка коллбека
    void Toggle::setCallback(std::function<void(bool)> callback) {
        onToggle = std::move(callback);
    }

    // Изменение состояния
    void Toggle::setToggled(bool value) {
        isToggled = value;
        updateVisuals();
    }

    // Получение состояния
    bool Toggle::getToggled() const {
        return isToggled;
    }
}
