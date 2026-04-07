#include "Projectile.hpp"
#include "ResourceManager.hpp"
#include <cmath>

Projectile::Projectile(sf::Vector2f startPos, Enemy* targetEnemy, int dmg, float spd, int splash, std::string slug)
    : pos(startPos), target(targetEnemy), damage(dmg), speed(spd), splashRadius(splash), typeSlug(slug) {
}

void Projectile::update(float deltaTime, std::vector<Enemy>& enemies) {
    // если цель погибла до прилета снаряда, снаряд исчезает
    if (!target || !target->isAlive()) {
        alive = false;
        return;
    }

    // вычисляем вектор к центру врага (враг 32x32, его центр +16)
    sf::Vector2f targetCenter = target->getPos() + sf::Vector2f(32.f, 32.f);
    sf::Vector2f dir = targetCenter - pos;
    float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);

    // расстояние, которое снаряд преодолеет за этот кадр
    float moveStep = speed * deltaTime;

    // Если расстояние меньше шага — значит в этом кадре точно долетает
    if (dist <= moveStep) {
        if (splashRadius > 0) {
            // урон по области для пушек
            for (auto& e : enemies) {
                if (!e.isAlive()) continue;
                sf::Vector2f ePos = e.getPos();
                sf::Vector2f diff = ePos - target->getPos();
                float d = std::sqrt(diff.x * diff.x + diff.y * diff.y);
                if (d <= (float)splashRadius) e.takeDamage(damage);
            }
        } else {
            // точечный урон
            target->takeDamage(damage);
        }
        alive = false;
    } else {
        // полет снаряда
        dir /= dist;
        pos += dir * moveStep;
    }
}

void Projectile::render(sf::RenderWindow& window, sf::Vector2f mapOffset) {
    // динамически формируем имя текстуры: tower-basic-proj
    sf::Sprite sprite(ResourceManager::get("tower-" + typeSlug + "-proj"));
    sprite.setOrigin({ 64.f, 64.f }); // центр текстуры 128x128
    sprite.setScale({ 0.07f, 0.07f }); // сжимаем до 9 пикселей
    sprite.setPosition(pos + mapOffset);
    window.draw(sprite);
}