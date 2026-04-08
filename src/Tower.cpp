#include "Tower.hpp"
#include "Enemy.hpp"
#include "ResourceManager.hpp"
#include "Colors.hpp"
#include "Projectile.hpp"

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

void Tower::update(float deltaTime, std::list<std::shared_ptr<Enemy>>& enemies, std::vector<Projectile>& projectiles, sf::Vector2f mapOffset) {
    // Логический центр башни на карте
    sf::Vector2f towerMapPos = sf::Vector2f(gridPos * 64) + sf::Vector2f(32.f, 32.f);

    std::shared_ptr<Enemy> targetShared = nullptr; // храним указатель на цель
    int maxPathIndex = -1;

    // Поиск цели
    for (auto& e : enemies) {
        if (!e->isAlive()) continue;
        sf::Vector2f diff = e->getPos() - towerMapPos;
        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        if (dist <= stats.range && e->getPathIndex() > maxPathIndex) {
            maxPathIndex = e->getPathIndex();
            targetShared = e;
        }
    }

    if (targetShared) {
        // текущие данные врага
        sf::Vector2f enemyPos = targetShared->getPos();
        sf::Vector2f enemyVel = targetShared->getVelocity();

        // расстояние до врага для расчета времени полета снаряда
        sf::Vector2f currentDiff = enemyPos - towerMapPos;
        float currentDist = std::sqrt(currentDiff.x * currentDiff.x + currentDiff.y * currentDiff.y);

        float projectileSpeed = 600.f;
        float timeToHit = currentDist / projectileSpeed;

        // предсказание
        sf::Vector2f predictedPos = enemyPos + sf::Vector2f(32.f, 32.f) + (enemyVel * timeToHit);

        sf::Vector2f aimDiff = predictedPos - towerMapPos;
        float targetDeg = std::atan2(aimDiff.y, aimDiff.x) * (180.f / 3.14159f) + 90.f;

        float angleDiff = targetDeg - currentAngle;
        while (angleDiff > 180.f)  angleDiff -= 360.f;
        while (angleDiff < -180.f) angleDiff += 360.f;

        float step = rotationSpeed * deltaTime;
        if (std::abs(angleDiff) <= step) {
            currentAngle = targetDeg;
        } else {
            currentAngle += (angleDiff > 0 ? step : -step);
        }

        currentAngle = fmod(currentAngle, 360.f);
        if (currentAngle < 0) currentAngle += 360.f;

        fireTimer += deltaTime;
        if (fireTimer >= 1.f / stats.firerate) {
            if (std::abs(angleDiff) < 10.f) {
                fireTimer = 0.f;

                float muzzleOffset = 26.f;
                float rad = (currentAngle - 90.f) * (3.14159f / 180.f);

                sf::Vector2f muzzlePos;
                muzzlePos.x = towerMapPos.x + std::cos(rad) * muzzleOffset;
                muzzlePos.y = towerMapPos.y + std::sin(rad) * muzzleOffset;

                // Передаем указатель на цель
                projectiles.emplace_back(muzzlePos, targetShared, stats.damage, 600.f, stats.splash, typeSlug);
            }
        }
    }
}

sf::Vector2i Tower::getGridPos() const { return gridPos; }