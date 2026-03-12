#include "Base.hpp"

Base::Base(sf::Vector2i pos) : basePos(pos) {

}

void Base::takeDamage(int damage) {
    baseLives -= damage;
    if (baseLives <= 0) {
        baseLives = 0;
        destroyed = true;
    }
}

int Base::getLives() const {
	return baseLives;
}

bool Base::isDestroyed() const {
	return destroyed;
}
