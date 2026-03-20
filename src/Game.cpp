#include "Colors.hpp"
#include "Game.hpp"
#include "GameData.hpp"
#include "ResourceManager.hpp"
#include "WaveSystem.hpp"
#include <SFML/Graphics.hpp>
#include <Windows.h>
#include <algorithm>
#include <iostream>

Game::Game() : window(sf::VideoMode({ 1920, 1080 }), "Tower Defence", sf::Style::Default/*, sf::State::Fullscreen*/), base(map.getBasePos()), hud("assets/fonts/web_ibm_mda.ttf") {
    window.setFramerateLimit(60);
    window.setMinimumSize(sf::Vector2u({ 1280, 720 }));
    
    // загрузка иконок
    ResourceManager::load("icon-pause", "assets/icons/pause.png");
    ResourceManager::load("icon-skip", "assets/icons/play.png");
    ResourceManager::load("icon-coins", "assets/icons/coins.png");
    ResourceManager::load("icon-heart", "assets/icons/heart.png");

    // загрузка статы врагов и башен
    GameData::load();

    // загрузка спрайтов тайлов
    ResourceManager::load("road", "assets/sprites/tile-road.png");
    ResourceManager::load("platform", "assets/sprites/tile-platform.png");
    ResourceManager::load("portal", "assets/sprites/tile-portal.png");
    ResourceManager::load("base", "assets/sprites/tile-base.png");
    ResourceManager::load("active", "assets/sprites/tile-active-layer.png");

    // загрузка спрайтов врагов
    ResourceManager::load("enemy-basic", "assets/sprites/enemy-basic.png");
    ResourceManager::load("enemy-fast", "assets/sprites/enemy-fast.png");
    ResourceManager::load("enemy-strong", "assets/sprites/enemy-strong.png");

    // загрузка спрайтов башен
    ResourceManager::load("tower-basic-base", "assets/sprites/tower-basic-base.png");
    ResourceManager::load("tower-basic-turret", "assets/sprites/tower-basic-turret.png");
    ResourceManager::load("tower-cannon-base", "assets/sprites/tower-cannon-base.png");
    ResourceManager::load("tower-cannon-turret", "assets/sprites/tower-cannon-turret.png");
    ResourceManager::load("tower-double-base", "assets/sprites/tower-double-base.png");
    ResourceManager::load("tower-double-turret", "assets/sprites/tower-double-turret.png");
    ResourceManager::load("tower-sniper-base", "assets/sprites/tower-sniper-base.png");
    ResourceManager::load("tower-sniper-turret", "assets/sprites/tower-sniper-turret.png");

    // загрузка превью башен
    ResourceManager::load("tower-basic-preview", "assets/sprites/tower-basic-preview.png");
    ResourceManager::load("tower-cannon-preview", "assets/sprites/tower-cannon-preview.png");
    ResourceManager::load("tower-double-preview", "assets/sprites/tower-double-preview.png");
    ResourceManager::load("tower-sniper-preview", "assets/sprites/tower-sniper-preview.png");

    waveSystem.loadWaves("data/levels/level01.map");
    map.centerOnScreen(window.getSize(), 75.f, 120.f);
    money = map.getStartMoney();
}

void Game::run() {
    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        handleEvents();
        update(deltaTime);
        render();
    }
}

void Game::handleEvents() {
    while (std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }

        if (const auto* key = event->getIf<sf::Event::KeyPressed>())
            if (key->code == sf::Keyboard::Key::Space)
                waveSystem.startWave();

        if (const auto* key = event->getIf<sf::Event::KeyPressed>())
            if (key->code == sf::Keyboard::Key::Escape)
                window.close();

        if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            sf::Vector2u newSize = { resized->size.x, resized->size.y };
            map.centerOnScreen(newSize, 75.f, 120.f);

            // обновляем View чтобы SFML не растягивал картинку
            sf::FloatRect visibleArea({ 0.f, 0.f }, sf::Vector2f(newSize));
            window.setView(sf::View(visibleArea));
        }

        if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>()) {
            sf::Vector2f mousePos = sf::Vector2f(click->position);
            hud.handleClick(mousePos);

            if (click->button == sf::Mouse::Button::Left) {
                Tile* tile = map.getTileAtScreen(mousePos);
                if (tile && tile->type == TileType::Platform) {
                    int slot = hud.getSelectedSlot();
                    if (slot != -1) {
                        std::vector<std::string> names = { "basic", "cannon", "double", "sniper" };
                        int cost = GameData::getTower(names[slot]).cost;
                        
                        // проверяем нет ли уже башни
                        bool occupied = false;
                        for (auto& t : towers)
                            if (t.getGridPos() == tile->gridPos) { occupied = true; break; }

                        if (!occupied) {
                            std::vector<std::string> names = { "basic", "cannon", "double", "sniper" };
                            TowerType types[] = { TowerType::Basic, TowerType::Cannon, TowerType::Double, TowerType::Sniper };
                            int cost = GameData::getTower(names[slot]).cost;
                            if (money >= cost) {
                                money -= cost;
                                towers.emplace_back(types[slot], tile->gridPos);
                                hud.resetSelectedSlot();
                            }
                        }
                    } else {
                        map.setSelectedTile(mousePos);
                    }
                } else {
                    map.setSelectedTile(mousePos);
                }
            }
        }
    }
}

void Game::render() {
    window.clear(sf::Color(27, 27, 27));
    map.render(window);
    hud.render(window, money, base.getLives(), waveSystem.getCurrentWave(), waveSystem.getState());

    Tile* selected = map.getSelectedTile();
    for (auto& tower : towers) {
        bool showRadius = selected && selected->gridPos == tower.getGridPos();
        tower.render(window, map.getMapOffset(), showRadius);
    }

    for (auto& enemy : enemies)
        enemy.render(window, map.getMapOffset());

    window.display();
}

void Game::update(float deltaTime) {
    waveSystem.update(deltaTime, enemies, map.getPath());
    for (auto& enemy : enemies)
        enemy.update(deltaTime);

    for (auto& tower : towers)
        tower.update(deltaTime, enemies, map.getMapOffset());

    // проверяем кто достиг базы — до удаления
    for (auto& enemy : enemies)
        if (enemy.hasReachedBase())
            base.takeDamage(1);

    for (auto& enemy : enemies)
        if (enemy.isKilled())
            money += GameData::getEnemy(enemy.getType()).reward;

    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [](const Enemy& e) { return !e.isAlive(); }),
        enemies.end()
    );
}