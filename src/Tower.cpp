#include "Tower.hpp"
#include "Enemy.hpp"
#include "ResourceManager.hpp"
#include "Colors.hpp"

Tower::Tower(TowerType type, sf::Vector2i gridPos)
    : type(type), gridPos(gridPos) {
    // загружаем статы по типу башни
    std::string name;
    switch (type) {
        case TowerType::Basic:  name = "basic";  break;
        case TowerType::Cannon: name = "cannon"; break;
        case TowerType::Double: name = "double"; break;
        case TowerType::Sniper: name = "sniper"; break;
    }
    stats = GameData::getTower(name);
}

void Tower::render(sf::RenderWindow& window, sf::Vector2f mapOffset, bool showRadius) {
    // пиксельная позиция тайла башни
    sf::Vector2f pixelPos = sf::Vector2f(gridPos * 64) + mapOffset;

    // определяем имя текстуры по типу башни
    std::string typeName;
    switch (type) {
    case TowerType::Basic:  typeName = "tower-basic";  break;
    case TowerType::Cannon: typeName = "tower-cannon"; break;
    case TowerType::Double: typeName = "tower-double"; break;
    case TowerType::Sniper: typeName = "tower-sniper"; break;
    }

    if (showRadius) {
        sf::CircleShape radiusCircle(stats.range);
        radiusCircle.setFillColor(Colors::radiusFill);
        radiusCircle.setOutlineColor(Colors::radiusOutline);
        radiusCircle.setOutlineThickness(1.f);
        radiusCircle.setOrigin({ stats.range, stats.range });
        radiusCircle.setPosition(pixelPos + sf::Vector2f(32.f, 32.f));
        window.draw(radiusCircle);
    }

    // рисуем основание — не вращается
    sf::Sprite base(ResourceManager::get(typeName + "-base"));
    base.setPosition(pixelPos);
    window.draw(base);

    // рисуем турель — вращается вокруг центра тайла
    sf::Sprite turret(ResourceManager::get(typeName + "-turret"));
    turret.setOrigin({ 32.f, 32.f });                          // центр спрайта 64x64
    turret.setPosition(pixelPos + sf::Vector2f(32.f, 32.f));   // центр тайла
    turret.setRotation(angle);
    window.draw(turret);
}

void Tower::update(float deltaTime, std::vector<Enemy>& enemies, sf::Vector2f mapOffset) {
    // пиксельный центр башни
    sf::Vector2f towerCenter = sf::Vector2f(gridPos * 64) + mapOffset + sf::Vector2f(32.f, 32.f);

    // ищем цель по индексу
    enemyIndex = -1;
    int maxPathIndex = -1;

    for (int i = 0; i < (int)enemies.size(); i++) {
        if (!enemies[i].isAlive()) continue;
        sf::Vector2f ePos = enemies[i].getPos() + mapOffset;
        sf::Vector2f diff = ePos - towerCenter;
        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        if (dist <= stats.range && enemies[i].getPathIndex() > maxPathIndex) {
            maxPathIndex = enemies[i].getPathIndex();
            enemyIndex = i;
        }
    }

    if (enemyIndex != -1) {
        Enemy& target = enemies[enemyIndex];
        sf::Vector2f ePos = target.getPos() + mapOffset;
        sf::Vector2f diff = ePos - towerCenter;

        float deg = std::atan2(diff.y, diff.x) * (180.f / 3.14159f) + 90.f;

        // плавный поворот
        float angleDiff = deg - currentAngle;
        while (angleDiff > 180.f)  angleDiff -= 360.f;
        while (angleDiff < -180.f) angleDiff += 360.f;
        float step = rotationSpeed * deltaTime;
        if (std::abs(angleDiff) <= step)
            currentAngle = deg;
        else
            currentAngle += (angleDiff > 0 ? step : -step);
        angle = sf::degrees(currentAngle);

        fireTimer += deltaTime;
        if (fireTimer >= 1.f / stats.firerate) {
            fireTimer = 0.f;
            target.takeDamage(stats.damage);

            if (stats.splash > 0) {
                for (auto& e : enemies) {
                    if (!e.isAlive() || &e == &target) continue;
                    sf::Vector2f diff2 = e.getPos() + mapOffset - ePos;
                    float dist = std::sqrt(diff2.x * diff2.x + diff2.y * diff2.y);
                    if (dist <= (float)stats.splash)
                        e.takeDamage(stats.damage);
                }
            }
        }
    }
}

sf::Vector2i Tower::getGridPos() const { return gridPos; }