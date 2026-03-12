#include "Game.hpp"
#include "ResourceManager.hpp"
#include <algorithm>
#include <iostream>
#include <SFML/Graphics.hpp>

Game::Game() : window(sf::VideoMode({ 1280, 720 }), "Tower Defence"), base(map.getBasePos()) {
    window.setFramerateLimit(60);
  
    // тайлы
    ResourceManager::load("road", "assets/sprites/tile-road.png");
    ResourceManager::load("platform", "assets/sprites/tile-platform.png");
    ResourceManager::load("portal", "assets/sprites/tile-portal.png");
    ResourceManager::load("base", "assets/sprites/tile-base.png");
    ResourceManager::load("active", "assets/sprites/tile-active-layer.png");

    // враги
    ResourceManager::load("enemy-basic", "assets/sprites/enemy-basic.png");
    ResourceManager::load("enemy-fast", "assets/sprites/enemy-fast.png");
    ResourceManager::load("enemy-strong", "assets/sprites/enemy-strong.png");

    // башни
    ResourceManager::load("tower-basic-base", "assets/sprites/tower-basic-base.png");
    ResourceManager::load("tower-basic-turret", "assets/sprites/tower-basic-turret.png");
    ResourceManager::load("tower-cannon-base", "assets/sprites/tower-cannon-base.png");
    ResourceManager::load("tower-cannon-turret", "assets/sprites/tower-cannon-turret.png");
    ResourceManager::load("tower-double-base", "assets/sprites/tower-double-base.png");
    ResourceManager::load("tower-double-turret", "assets/sprites/tower-double-turret.png");
    ResourceManager::load("tower-sniper-base", "assets/sprites/tower-sniper-base.png");
    ResourceManager::load("tower-sniper-turret", "assets/sprites/tower-sniper-turret.png");

    waveSystem.loadWaves("data/levels/level01.map");
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
    }
}

void Game::render() {
    window.clear(sf::Color(27, 27, 27));

    map.render(window);
    for (auto& enemy : enemies)
        enemy.render(window);

    window.display();
}

void Game::update(float deltaTime) {
    waveSystem.update(deltaTime, enemies, map.getPath());
    for (auto& enemy : enemies)
        enemy.update(deltaTime);

    // проверяем кто достиг базы — до удаления
    for (auto& enemy : enemies)
        if (enemy.hasReachedBase())
            base.takeDamage(1);

    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [](const Enemy& e) { return !e.isAlive(); }),
        enemies.end()
    );
}