#include "Tower.hpp"
#include "Enemy.hpp"
#include "ResourceManager.hpp"
#include "Colors.hpp"
#include "Projectile.hpp"
#include "UpgradeManager.hpp"
#include "utils/Math.hpp"

// Конструктор башни
Tower::Tower(const std::string& slug, sf::Vector2i gridPos, UpgradeManager& upgradeManager)
    : typeSlug(slug), gridPos(gridPos) {

    textureBase = &ResourceManager::get("tower-" + typeSlug + "-base");
    textureTower = &ResourceManager::get("tower-" + typeSlug + "-turret");

    // базовые статы из GameData
    auto baseStats = GameData::getTower(slug);
    stats = baseStats;

    // применяем улучшения
    stats.damage = (int)upgradeManager.getDamage(slug);
    stats.firerate = upgradeManager.getFirerate(slug);
    stats.range = upgradeManager.getRange(slug);
}

// Обновление башни
void Tower::update(float deltaTime, std::vector<std::unique_ptr<Enemy>>& enemies, std::vector<Projectile>& projectiles, sf::Vector2f mapOffset) {
    sf::Vector2f towerMapPos = sf::Vector2f(gridPos * 64) + sf::Vector2f(32.f, 32.f);

    int maxPathIndex = -1;
    float rangeSq = stats.range * stats.range;

    Enemy* targetPtr = nullptr;

    // поиск ближайшей цели в радиусе
    for (auto& e : enemies) {
        float dSq = Math::getDistSq(e->getPos(), towerMapPos);

        if (dSq <= rangeSq && e->getPathIndex() > maxPathIndex) {
            maxPathIndex = e->getPathIndex();
            targetPtr = e.get();
        }
    }

    if (targetPtr) {
        sf::Vector2f enemyPos = targetPtr->getPos();
        sf::Vector2f enemyVel = targetPtr->getVelocity();

        // расстояние до цели
        sf::Vector2f currentDiff = enemyPos - towerMapPos;
        float currentDist = std::sqrt(currentDiff.x * currentDiff.x + currentDiff.y * currentDiff.y);

        // время полёта снаряда
        float projectileSpeed = 600.f;
        float timeToHit = currentDist / projectileSpeed;

        // предсказание позиции врага
        sf::Vector2f predictedPos = enemyPos + sf::Vector2f(32.f, 32.f) + (enemyVel * timeToHit);

        // расчёт целевого угла
        sf::Vector2f aimDiff = predictedPos - towerMapPos;
        float targetDeg = Math::getAngle(aimDiff);

        // разница текущего и целевого угла
        float angleDiff = targetDeg - currentAngle;
        while (angleDiff > 180.f)  angleDiff -= 360.f;
        while (angleDiff < -180.f) angleDiff += 360.f;

        // плавный поворот турели к цели
        float step = rotationSpeed * deltaTime;
        if (std::abs(angleDiff) <= step) {
            currentAngle = targetDeg;
        } else {
            currentAngle += (angleDiff > 0 ? step : -step);
        }

        // нормализация угла
        currentAngle = fmod(currentAngle, 360.f);
        if (currentAngle < 0) currentAngle += 360.f;

        // стрельба при наведении
        fireTimer += deltaTime;
        if (fireTimer >= 1.f / stats.firerate) {
            if (std::abs(angleDiff) < 10.f) {
                fireTimer = 0.f;

                // позиция дула
                float muzzleOffset = 26.f;
                float rad = (currentAngle - 90.f) * (3.14159f / 180.f);

                sf::Vector2f muzzlePos;
                muzzlePos.x = towerMapPos.x + std::cos(rad) * muzzleOffset;
                muzzlePos.y = towerMapPos.y + std::sin(rad) * muzzleOffset;

                projectiles.emplace_back(muzzlePos, targetPtr, stats.damage, 600.f, stats.splash, typeSlug);
            }
        }
    }
}

// Отрисовка башни
void Tower::render(sf::RenderWindow& window, sf::Vector2f mapOffset, bool showRadius) {
    sf::Vector2f pixelPos = sf::Vector2f(gridPos * 64) + mapOffset;

    // радиус атаки при наведении
    if (showRadius) {
        sf::CircleShape radiusCircle(stats.range);
        radiusCircle.setFillColor(Colors::Theme::RadiusFill);
        radiusCircle.setOutlineColor(Colors::Theme::RadiusOutline);
        radiusCircle.setOutlineThickness(1.f);
        radiusCircle.setOrigin({ stats.range, stats.range });
        radiusCircle.setPosition(pixelPos + sf::Vector2f(32.f, 32.f));
        window.draw(radiusCircle);
    }

    // основание башни
    sf::Sprite base(*textureBase);
    base.setScale({ 0.125f, 0.125f });
    base.setPosition(pixelPos);
    window.draw(base);

    // турель с поворотом
    sf::Sprite turret(*textureTower);
    turret.setScale({ 0.125f, 0.125f });
    turret.setOrigin({ 256.f, 256.f });
    turret.setPosition(pixelPos + sf::Vector2f(32.f, 32.f));
    turret.setRotation(sf::degrees(currentAngle));
    window.draw(turret);
}

// Получение позиции на сетке
sf::Vector2i Tower::getGridPos() const {
    return gridPos;
}

// Возвращает строковый идентификатор типа башни
std::string Tower::getTypeSlug() const {
    return typeSlug;
}

// Возвращает исходную стоимость постройки башни
int Tower::getCost() const {
    return stats.cost;
}
