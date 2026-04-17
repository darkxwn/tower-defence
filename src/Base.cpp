#include "Base.hpp"

// Конструктор базы
Base::Base(sf::Vector2i pos) : basePos(pos) {}

// Получение урона
void Base::takeDamage(int damage) {
    baseLives -= damage;
    if (baseLives <= 0) {
        baseLives = 0;
        destroyed = true;
    }
}

// Получение количества жизней
int Base::getLives() const {
	return baseLives;
}

// Проверка состояния разрушения
bool Base::isDestroyed() const {
	return destroyed;
}
