#include "ui/Slider.hpp"
#include "Colors.hpp"
#include <algorithm>
#include <cmath>

namespace UI {

    // Конструктор слайдера
    Slider::Slider(const sf::Font& font, float min, float max, float initial, sf::Vector2f size)
        : minValue(min), maxValue(max), currentValue(initial) {
        this->size = size;
    
        // настройка дорожки
        track.setSize({ size.x * 0.8f, size.y * 0.2f }); // 80% ширины под дорожку
        track.setFillColor(Colors::Theme::WidgetBg);
    
        // настройка ручки
        handle.setSize({ size.y * 0.6f, size.y });
        handle.setFillColor(sf::Color::White);

        // настройка текста значения
        valueText = std::make_unique<sf::Text>(font);
        valueText->setCharacterSize(20);
        valueText->setFillColor(Colors::Theme::TextMain);
    
        updateInternalLayout();
    }

    // Обновление положения ручки и текста на основе значения
    void Slider::updateInternalLayout() {
        float range = maxValue - minValue;
        if (range <= 0.f) return;
    
        float trackWidth = size.x * 0.8f;
        float percent = (currentValue - minValue) / range;
        float handleX = percent * (trackWidth - handle.getSize().x);
    
        track.setPosition({ position.x, position.y + (size.y - track.getSize().y) / 2.f });
        handle.setPosition({ position.x + handleX, position.y });

        // обновление текста с учётом precision
        if (valueText) {
            char buf[16];
            if (precision == 0) {
                snprintf(buf, sizeof(buf), "%.0f", currentValue);
            } else {
                snprintf(buf, sizeof(buf), "%.*f", precision, currentValue);
            }
            valueText->setString(buf);
        }
        sf::FloatRect textBounds = valueText->getLocalBounds();
        valueText->setOrigin({ textBounds.position.x, textBounds.position.y + textBounds.size.y / 2.f });
        valueText->setPosition({ position.x + trackWidth + 15.f, position.y + size.y / 2.f });
    }

    // Обработка событий ввода
    void Slider::handleEvent(const sf::Event& event, const sf::RenderWindow& window, const sf::View& uiView) {
        if (!enabled || !visible) return;

        sf::Vector2i pixelPos;
        bool interactionOccurred = false;
        bool isPress = false;
        bool isRelease = false;
        bool isMove = false;

        if (const auto* mousePress = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mousePress->button == sf::Mouse::Button::Left) {
                pixelPos = mousePress->position;
                isPress = true;
                interactionOccurred = true;
            }
        }
        else if (const auto* touchStart = event.getIf<sf::Event::TouchBegan>()) {
            pixelPos = touchStart->position;
            isPress = true;
            interactionOccurred = true;
        }
        else if (const auto* mouseRelease = event.getIf<sf::Event::MouseButtonReleased>()) {
            if (mouseRelease->button == sf::Mouse::Button::Left) {
                pixelPos = mouseRelease->position;
                isRelease = true;
                interactionOccurred = true;
            }
        }
        else if (const auto* touchEnd = event.getIf<sf::Event::TouchEnded>()) {
            pixelPos = touchEnd->position;
            isRelease = true;
            interactionOccurred = true;
        }
        else if (const auto* mouseMove = event.getIf<sf::Event::MouseMoved>()) {
            pixelPos = mouseMove->position;
            isMove = true;
            interactionOccurred = true;
        }
        else if (const auto* touchMove = event.getIf<sf::Event::TouchMoved>()) {
            pixelPos = touchMove->position;
            isMove = true;
            interactionOccurred = true;
        }

        if (!interactionOccurred) return;

        sf::Vector2f mousePos = window.mapPixelToCoords(pixelPos, uiView);
        float trackWidth = size.x * 0.8f;
        sf::FloatRect interactiveArea({ position.x, position.y }, { trackWidth, size.y });

        auto updateFromMouse = [&](float x) {
            float maxHandleX = trackWidth - handle.getSize().x;
            if (maxHandleX <= 0.f) return; // защита от некорректного размера

            float relativeX = x - position.x - handle.getSize().x / 2.f;
            float percent = std::clamp(relativeX / maxHandleX, 0.f, 1.f);
            float newValue = minValue + percent * (maxValue - minValue);
        
            if (newValue != currentValue) {
                currentValue = newValue;
                if (onValueChange) onValueChange(currentValue);
                updateInternalLayout();
            }
        };

        if (isPress) {
            if (interactiveArea.contains(mousePos)) {
                isDragging = true;
                updateFromMouse(mousePos.x);
            }
        }
        else if (isRelease) {
            isDragging = false;
        }
        else if (isMove && isDragging) {
            updateFromMouse(mousePos.x);
        }
    }

    // Отрисовка слайдера
    void Slider::render(sf::RenderWindow& window) const {
        if (!visible) return;

        window.draw(track);
        window.draw(handle);
        if (valueText) window.draw(*valueText);
    }

    // Получение глобальных границ
    sf::FloatRect Slider::getGlobalBounds() const {
        return { position, size };
    }

    // Изменение позиции
    void Slider::setPosition(sf::Vector2f pos) {
        position = pos;
        updateInternalLayout();
    }

    // Изменение размера
    void Slider::setSize(sf::Vector2f size) {
        this->size = size;
        track.setSize({ size.x * 0.8f, size.y * 0.2f });
        handle.setSize({ size.y * 0.6f, size.y });
        updateInternalLayout();
    }

    // Получение значения
    float Slider::getValue() const {
        return currentValue;
    }

    // Изменение значения
    void Slider::setValue(float value) {
        currentValue = std::clamp(value, minValue, maxValue);
        updateInternalLayout();
    }

    // Изменение обратного вызова
    void Slider::setCallback(std::function<void(float)> callback) {
        onValueChange = std::move(callback);
    }

    // Изменение цвета дорожки
    void Slider::setTrackColor(sf::Color color) {
        trackColor = color;
        track.setFillColor(color);
    }

    // Изменение цвета ручки
    void Slider::setHandleColor(sf::Color color) {
        handleColor = color;
        handle.setFillColor(color);
    }

    // Изменение количества дробных знаков у текста
    void Slider::setPrecision(unsigned int value) {
        precision = value;
    }

}
