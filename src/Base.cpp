#include "Base.hpp"

// Конструктор базы
Base::Base(sf::Vector2i pos, int lives) : baseLives(lives) {
    basePos = sf::Vector2f(pos.x * 64.f, pos.y * 64.f);
}

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
