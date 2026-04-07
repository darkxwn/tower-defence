#include "Tower.hpp"
#include "Enemy.hpp"
#include "ResourceManager.hpp"
#include "Colors.hpp"
#include <Projectile.hpp>

Tower::Tower(const std::string& slug, sf::Vector2i gridPos)
    : typeSlug(slug), gridPos(gridPos) {
    // загружаем статы напрямую по имени из конфига
    stats = GameData::getTower(slug);
}

void Tower::render(sf::RenderWindow& window, sf::Vector2f mapOffset, bool showRadius) {
    sf::Vector2f pixelPos = sf::Vector2f(gridPos * 64) + mapOffset;

    if (showRadius) {
        sf::CircleShape radiusCircle(stats.range);
        radiusCircle.setFillColor(Colors::radiusFill);
        radiusCircle.setOutlineColor(Colors::radiusOutline);
        radiusCircle.setOutlineThickness(1.f);
        radiusCircle.setOrigin({ stats.range, stats.range });
        radiusCircle.setPosition(pixelPos + sf::Vector2f(32.f, 32.f));
        window.draw(radiusCircle);
    }

    // отрисовка основания: tower-basic-base
    sf::Sprite base(ResourceManager::get("tower-" + typeSlug + "-base"));
    base.setScale({ 0.125f, 0.125f });
    base.setPosition(pixelPos);
    window.draw(base);

    // отрисовка турели: tower-basic-turret
    sf::Sprite turret(ResourceManager::get("tower-" + typeSlug + "-turret"));
    turret.setScale({ 0.125f, 0.125f });
    turret.setOrigin({ 256.f, 256.f });
    turret.setPosition(pixelPos + sf::Vector2f(32.f, 32.f));
    turret.setRotation(sf::degrees(currentAngle));
    window.draw(turret);
}

void Tower::update(float deltaTime, std::vector<Enemy>& enemies, std::vector<Projectile>& projectiles, sf::Vector2f mapOffset) {
    // Логический центр башни на карте (БЕЗ mapOffset) 
    sf::Vector2f towerMapPos = sf::Vector2f(gridPos * 64) + sf::Vector2f(32.f, 32.f);

    enemyIndex = -1;
    int maxPathIndex = -1;

    for (int i = 0; i < (int)enemies.size(); i++) {
        if (!enemies[i].isAlive()) continue;

        // Расстояние считаем в координатах карты 
        sf::Vector2f diff = enemies[i].getPos() - towerMapPos;
        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

        if (dist <= stats.range && enemies[i].getPathIndex() > maxPathIndex) {
            maxPathIndex = enemies[i].getPathIndex();
            enemyIndex = i;
        }
    }

    if (enemyIndex != -1) {
        Enemy& target = enemies[enemyIndex];
        sf::Vector2f diff = target.getPos() - towerMapPos;
        float deg = std::atan2(diff.y, diff.x) * (180.f / 3.14159f) + 90.f;

        // Плавный поворот 
        float angleDiff = deg - currentAngle;
        while (angleDiff > 180.f)  angleDiff -= 360.f;
        while (angleDiff < -180.f) angleDiff += 360.f;

        float step = rotationSpeed * deltaTime;
        if (std::abs(angleDiff) <= step) currentAngle = deg;
        else currentAngle += (angleDiff > 0 ? step : -step);

        // Таймер стрельбы 
        fireTimer += deltaTime;
        if (fireTimer >= 1.f / stats.firerate) {
            // Спавним снаряд в координатах карты 
            if (std::abs(angleDiff) < 15.f) {
                fireTimer = 0.f;
                projectiles.emplace_back(towerMapPos, &target, stats.damage, 600.f, stats.splash, typeSlug);
            }
        }
    }
}

sf::Vector2i Tower::getGridPos() const { return gridPos; }