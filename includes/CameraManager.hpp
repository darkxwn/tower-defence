#pragma once
#include <SFML/Graphics.hpp>

class CameraManager {
private:
    sf::View worldView; // Камера для карты, башен, врагов
    sf::View uiView;    // Камера для интерфейса (фиксированная)

    float currentZoom = 1.0f;
    const float baseUiHeight = 1080.f; // Логическая высота интерфейса

public:
    CameraManager() = default;

    // Обновляет обе камеры при изменении размера окна
    void update(sf::Vector2u windowSize) {
        float sw = static_cast<float>(windowSize.x);
        float sh = static_cast<float>(windowSize.y);
        float aspect = sw / sh;

        // --- Настройка UI камеры ---
        // Фиксируем высоту, ширина адаптируется. (0,0) всегда в левом верхнем углу.
        float uiWidth = baseUiHeight * aspect;
        uiView = sf::View(sf::FloatRect({ 0.f, 0.f }, { uiWidth, baseUiHeight }));

        // --- Настройка World камеры ---
        // Чтобы карта на Android не была крошечной, сделаем её масштаб 
        // зависимым от высоты UI, но с учетом зума.
        worldView = sf::View(sf::FloatRect({ 0.f, 0.f }, { uiWidth, baseUiHeight }));
        worldView.zoom(currentZoom);

        // Центрируем камеру мира изначально (потом можно будет двигать)
        worldView.setCenter({ uiWidth / 2.f, baseUiHeight / 2.f });
    }

    // Применить камеру мира для отрисовки объектов
    void applyWorld(sf::RenderWindow& window) const {
        window.setView(worldView);
    }

    // Применить UI камеру для отрисовки HUD
    void applyUi(sf::RenderWindow& window) const {
        window.setView(uiView);
    }

    // Зуммирование
    void zoom(float factor) {
        currentZoom *= factor;
        worldView.zoom(factor);
    }

    // Перемещение камеры мира (панорамирование)
    void moveWorld(sf::Vector2f offset) {
        worldView.move(offset * currentZoom);
    }

    // Перевод пикселей в координаты UI
    sf::Vector2f mapPixelToUi(sf::Vector2i pixelPos, const sf::RenderWindow& window) const {
        return window.mapPixelToCoords(pixelPos, uiView);
    }

    // Перевод пикселей в координаты мира
    sf::Vector2f mapPixelToWorld(sf::Vector2i pixelPos, const sf::RenderWindow& window) const {
        return window.mapPixelToCoords(pixelPos, worldView);
    }

    // Получить размеры текущего интерфейса
    sf::Vector2f getUiSize() const {
        return uiView.getSize();
    }
};