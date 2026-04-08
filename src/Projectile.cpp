#include "Projectile.hpp"
#include "ResourceManager.hpp"
#include <cmath>

Projectile::Projectile(sf::Vector2f startPos, std::shared_ptr<Enemy> targetEnemy, int dmg, float spd, int splash, std::string slug)
    : pos(startPos), target(targetEnemy), damage(dmg), speed(spd), splashRadius(splash), typeSlug(slug) {
}

void Projectile::update(float deltaTime, std::list<std::shared_ptr<Enemy>>& enemies) {
    // если цель погибла до прилета снаряда, снаряд исчезает
    if (!target || !target->isAlive()) {
        alive = false;
        return;
    }

    // вычисляем вектор к центру врага (враг 32x32, его центр +16)
    sf::Vector2f targetCenter = target->getPos() + sf::Vector2f(32.f, 32.f);
    // пересчитываем направление (самонаведение)
    sf::Vector2f dir = targetCenter - pos;
    float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);

    // расстояние, которое снаряд преодолеет за этот кадр
    float moveStep = speed * deltaTime;

    // усли расстояние меньше шага — значит в этом кадре точно долетает
    if (dist <= moveStep) {
        // Наносим урон, только если враг еще не «убит» кем-то другим
        if (splashRadius > 0) {
            // логика сплэш урона
            for (auto& e : enemies) {
                if (e && e->isAlive()) {
                    sf::Vector2f ePos = e->getPos();
                    sf::Vector2f diff = ePos - target->getPos();
                    float d = std::sqrt(diff.x * diff.x + diff.y * diff.y);
                    if (d <= (float)splashRadius) {
                        e->takeDamage(damage);
                    }
                }
            }
        } else {// точечный урон
            target->takeDamage(damage);
        }
        alive = false;
    } else if (dist > 0.1f) {
        dir /= dist;
        pos += dir * moveStep;
    }
}

void Projectile::render(sf::RenderWindow& window, sf::Vector2f mapOffset) {
    // если цель потеряна, рисуем снаряд смотрящим вперед по последней позиции
    sf::Sprite sprite(ResourceManager::get("tower-" + typeSlug + "-proj"));
    sprite.setOrigin({ 64.f, 64.f });
    sprite.setScale({ 0.07f, 0.07f });
    sprite.setPosition(pos + mapOffset);

    if (target && target->isAlive()) {
        sf::Vector2f targetCenter = target->getPos() + sf::Vector2f(32.f, 32.f);
        sf::Vector2f diff = targetCenter - pos;
        float angle = std::atan2(diff.y, diff.x) * (180.f / 3.14159f) + 90.f;
        sprite.setRotation(sf::degrees(angle));
    }

    window.draw(sprite);
}