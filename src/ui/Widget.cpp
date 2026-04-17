#include "ui/Widget.hpp"

namespace UI {

    // Изменение позиции виджета
    void Widget::setPosition(sf::Vector2f pos) {
        position = pos;
    }

    // Получение текущей позиции
    sf::Vector2f Widget::getPosition() const {
        return position;
    }

    // Изменение размеров виджета
    void Widget::setSize(sf::Vector2f size) {
        this->size = size;
    }

    // Получение текущих размеров
    sf::Vector2f Widget::getSize() const {
        return size;
    }

    // Изменение состояния активности
    void Widget::setEnabled(bool enabled) {
        this->enabled = enabled;
    }

    // Получение состояния активности
    bool Widget::isEnabled() const {
        return enabled;
    }

    // Изменение видимости виджета
    void Widget::setVisible(bool visible) {
        this->visible = visible;
    }

    // Получение текущего статуса видимости
    bool Widget::isVisible() const {
        return visible;
    }

    // Изменение флага участия в автоматической раскладке
    void Widget::setFollowsLayout(bool value) {
        followsLayout = value;
    }

    // Получение текущего статуса участия в раскладке
    bool Widget::getFollowsLayout() const {
        return followsLayout;
    }

}
