#include "ui/Image.hpp"

namespace UI {
    // В SFML 3.0 sf::Sprite должен быть инициализирован текстурой в списке инициализации
    Image::Image(const sf::Texture& texture) : sprite(texture) {
        sf::FloatRect bounds = sprite.getLocalBounds();
        setSize({ bounds.size.x, bounds.size.y });
    }

    // Инициализация спрайта и установка заданного размера виджета
    Image::Image(const sf::Texture& texture, sf::Vector2f size) : sprite(texture) {
        setSize(size);
    }

    // Отрисовка спрайта, если виджет помечен как видимый
    void Image::render(sf::RenderWindow& window) const {
        if (visible) {
            window.draw(sprite);
        }
    }

    // Получение фактических границ спрайта на экране
    sf::FloatRect Image::getGlobalBounds() const {
        return sprite.getGlobalBounds();
    }

    // Синхронизация позиции спрайта с позицией виджета
    void Image::setPosition(sf::Vector2f pos) {
        Widget::setPosition(pos);
        sprite.setPosition(pos);
    }

    // Изменение размера виджета вызывает пересчет масштаба спрайта
    void Image::setSize(sf::Vector2f size) {
        Widget::setSize(size);
        updateScale();
    }

    // Смена текстуры "на лету" с автоматической подгонкой под текущий размер
    void Image::setTexture(const sf::Texture& texture) {
        sprite.setTexture(texture);
        updateScale();
    }

    // Установка дополнительного множителя масштаба
    void Image::setScale(sf::Vector2f scale) {
        customScale = scale;
        updateScale();
    }

    // Изменение цветового тона или прозрачности изображения
    void Image::setColor(sf::Color color) {
        sprite.setColor(color);
    }

    // Внутренний метод: вычисляет масштаб спрайта так, чтобы он вписался в size 
    // и учитывал customScale
    void Image::updateScale() {
        sf::FloatRect local = sprite.getLocalBounds();
        if (local.size.x > 0 && local.size.y > 0) {
            // Базовый масштаб для заполнения области size + множитель customScale
            sprite.setScale({ 
                (size.x / local.size.x) * customScale.x, 
                (size.y / local.size.y) * customScale.y 
            });
        }
    }
}
