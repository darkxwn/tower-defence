#include "Projectile.hpp"
#include "ResourceManager.hpp"
#include "utils/Math.hpp"

// Конструктор снаряда
Projectile::Projectile(sf::Vector2f startPos, Enemy* targetEnemy, int dmg, float spd, int splash, std::string slug)
    : pos(startPos), target(targetEnemy), damage(dmg), speed(spd), splashRadius(splash), typeSlug(slug) {
}

// Обновление снаряда
void Projectile::update(float deltaTime, std::vector<std::unique_ptr<Enemy>>& enemies) {
    // цель погибла до прилёта
    if (!target || !target->isAlive()) {
        alive = false;
        return;
    }

    // вектор к центру врага
    sf::Vector2f targetCenter = target->getPos() + sf::Vector2f(32.f, 32.f);
    sf::Vector2f diff = targetCenter - pos;

    float dist = Math::getLength(diff);
    // дистанция пролёта за кадр
    float moveStep = speed * deltaTime;

    // попадание, если расстояние меньше шага за кадр или минимального порога
    if (dist <= moveStep || dist < 10.f) {

        if (splashRadius > 0) {
            // урон по области через квадрат расстояния
            float splashSq = static_cast<float>(splashRadius * splashRadius);
            sf::Vector2f explosionPos = target->getPos();

            for (auto& e : enemies) {
                if (e && e->isAlive()) {
                    if (Math::getDistSq(e->getPos(), explosionPos) <= splashSq) {
                        e->takeDamage(damage);
                    }
                }
            }
        } else {
            // одиночный урон
            target->takeDamage(damage);
        }
        alive = false;
    } else {
        // движение к цели
        sf::Vector2f dir = diff / dist;
        pos += dir * moveStep;
    }
}

// Отрисовка снаряда
void Projectile::render(sf::RenderWindow& window, sf::Vector2f mapOffset) {
    // спрайт снаряда
    sf::Sprite sprite(ResourceManager::get("tower-" + typeSlug + "-proj"));
    sprite.setOrigin({ 64.f, 64.f });
    sprite.setScale({ 0.07f, 0.07f });
    sprite.setPosition(pos + mapOffset);

    // поворот к цели
    if (target && target->isAlive()) {
        sf::Vector2f targetCenter = target->getPos() + sf::Vector2f(32.f, 32.f);
        float angle = Math::getAngle(targetCenter - pos);
        sprite.setRotation(sf::degrees(angle));
    }

    window.draw(sprite);
}

// Проверка состояния жизни
bool Projectile::isAlive() const {
    return alive;
}