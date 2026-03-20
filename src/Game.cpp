#include "Colors.hpp"
#include "Game.hpp"
#include "GameData.hpp"
#include "ResourceManager.hpp"
#include "WaveSystem.hpp"
#include <SFML/Graphics.hpp>
#include <Windows.h>
#include <algorithm>
#include <iostream>

Game::Game() : window(sf::VideoMode({ 1920, 1080 }), "Tower Defence", sf::Style::Default), base(map.getBasePos()) {
    window.setFramerateLimit(60);
    window.setMinimumSize(sf::Vector2u({ 1280, 720 }));

    ResourceManager::loadFont("main", "assets/fonts/web_ibm_mda.ttf");

    ResourceManager::load("icon-pause", "assets/icons/pause.png");
    ResourceManager::load("icon-skip", "assets/icons/play.png");
    ResourceManager::load("icon-coins", "assets/icons/coins.png");
    ResourceManager::load("icon-heart", "assets/icons/heart.png");

    GameData::load();

    ResourceManager::load("road", "assets/sprites/tile-road.png");
    ResourceManager::load("platform", "assets/sprites/tile-platform.png");
    ResourceManager::load("portal", "assets/sprites/tile-portal.png");
    ResourceManager::load("base", "assets/sprites/tile-base.png");
    ResourceManager::load("active", "assets/sprites/tile-active-layer.png");

    ResourceManager::load("enemy-basic", "assets/sprites/enemy-basic.png");
    ResourceManager::load("enemy-fast", "assets/sprites/enemy-fast.png");
    ResourceManager::load("enemy-strong", "assets/sprites/enemy-strong.png");

    // загружаем спрайты башен автоматически по именам из GameData
    for (const auto& name : GameData::getTowerNames()) {
        ResourceManager::load("tower-" + name + "-base", "assets/sprites/tower-" + name + "-base.png");
        ResourceManager::load("tower-" + name + "-turret", "assets/sprites/tower-" + name + "-turret.png");
        ResourceManager::load("tower-" + name + "-preview", "assets/sprites/tower-" + name + "-preview.png");
    }

    waveSystem.loadWaves("data/levels/level01.map");
    map.centerOnScreen(window.getSize(), 75.f, 120.f);
    money = map.getStartMoney();
}

void Game::run() {
    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        handleEvents();
        if (gameState == GameState::Playing)
            update(deltaTime);
        render();
    }
}

void Game::handleEvents() {
    while (std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            window.close();

        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Space)
                waveSystem.startWave();

            if (key->code == sf::Keyboard::Key::Escape) {
                if (gameState == GameState::Paused)
                    gameState = GameState::Playing;
                else
                    window.close();
            }


            if (key->code == sf::Keyboard::Key::Escape)
                window.close();

            if (key->code == sf::Keyboard::Key::P) {
                if (gameState == GameState::Playing)
                    gameState = GameState::Paused;
                else if (gameState == GameState::Paused)
                    gameState = GameState::Playing;
            }


        }

        if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            sf::Vector2u newSize = { resized->size.x, resized->size.y };
            map.centerOnScreen(newSize, 75.f, 120.f);
            sf::FloatRect visibleArea({ 0.f, 0.f }, sf::Vector2f(newSize));
            window.setView(sf::View(visibleArea));
        }

        if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>()) {
            sf::Vector2f mousePos = sf::Vector2f(click->position);
            hud.handleClick(mousePos);

            // кнопка паузы в HUD
            if (hud.isPauseClicked()) {
                if (gameState == GameState::Playing)
                    gameState = GameState::Paused;
                else if (gameState == GameState::Paused)
                    gameState = GameState::Playing;
            }

            if (gameState == GameState::Paused && click->button == sf::Mouse::Button::Left) {
                if (pauseContinueBtn.getGlobalBounds().contains(mousePos)) {
                    gameState = GameState::Playing;
                }
                if (pauseRestartBtn.getGlobalBounds().contains(mousePos)) {
                    // перезапуск — пока просто закрываем, потом сделаем нормально
                    window.close();
                }
                if (pauseMenuBtn.getGlobalBounds().contains(mousePos)) {
                    // главное меню — пока просто закрываем
                    window.close();
                }
            }

            // кнопка скипа волны в HUD
            if (hud.isSkipClicked())
                waveSystem.startWave();

            if (click->button == sf::Mouse::Button::Left) {
                Tile* tile = map.getTileAtScreen(mousePos);
                if (tile && tile->type == TileType::Platform) {
                    int slot = hud.getSelectedSlot();
                    if (slot != -1) {
                        // получаем имя башни по индексу слота
                        auto towerNames = GameData::getTowerNames();
                        if (slot < (int)towerNames.size()) {
                            std::string name = towerNames[slot];
                            int cost = GameData::getTower(name).cost;

                            bool occupied = false;
                            for (auto& t : towers)
                                if (t.getGridPos() == tile->gridPos) { occupied = true; break; }

                            if (!occupied && money >= cost) {
                                // конвертируем имя в TowerType
                                TowerType type = TowerType::Basic;
                                if (name == "cannon") type = TowerType::Cannon;
                                else if (name == "double") type = TowerType::Double;
                                else if (name == "sniper") type = TowerType::Sniper;

                                money -= cost;
                                towers.emplace_back(type, tile->gridPos);
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
    window.clear(Colors::gameBg);
    map.render(window);
    hud.render(window, money, base.getLives(), waveSystem.getCurrentWave(), waveSystem.getState());

    Tile* selected = map.getSelectedTile();
    for (auto& tower : towers) {
        bool showRadius = selected && selected->gridPos == tower.getGridPos();
        tower.render(window, map.getMapOffset(), showRadius);
    }

    for (auto& enemy : enemies)
        enemy.render(window, map.getMapOffset());

    // оверлей паузы
    if (gameState == GameState::Paused) {
        auto& font = ResourceManager::getFont("main");
        auto winSize = window.getSize();
        float cx = winSize.x / 2.f;
        float cy = winSize.y / 2.f;

        // затемнение
        sf::RectangleShape overlay;
        overlay.setSize(sf::Vector2f(winSize));
        overlay.setFillColor(sf::Color(0, 0, 0, 150));
        window.draw(overlay);

        // текст ПАУЗА
        std::string pauseStr = "ПАУЗА";
        sf::Text pauseText(font, sf::String::fromUtf8(pauseStr.begin(), pauseStr.end()), 64);
        pauseText.setFillColor(sf::Color::White);
        pauseText.setPosition({ cx - pauseText.getLocalBounds().size.x / 2.f, cy - 180.f });
        window.draw(pauseText);

        // размер и отступ кнопок
        float btnW = 220.f, btnH = 60.f, btnGap = 30.f;
        float totalW = btnW * 3 + btnGap * 2;
        float btnY = cy - btnH / 2.f;

        // позиции кнопок
        pauseMenuBtn.setSize({ btnW, btnH });
        pauseMenuBtn.setFillColor(Colors::slotBg);
        pauseMenuBtn.setPosition({ cx - totalW / 2.f, btnY });
        window.draw(pauseMenuBtn);

        pauseRestartBtn.setSize({ btnW, btnH });
        pauseRestartBtn.setFillColor(Colors::slotBg);
        pauseRestartBtn.setPosition({ cx - btnW / 2.f, btnY });
        window.draw(pauseRestartBtn);

        pauseContinueBtn.setSize({ btnW, btnH });
        pauseContinueBtn.setFillColor(Colors::slotBg);
        pauseContinueBtn.setPosition({ cx + totalW / 2.f - btnW, btnY });
        window.draw(pauseContinueBtn);

        // тексты кнопок
        auto drawBtnText = [&](const std::string& str, sf::RectangleShape& btn) {
            sf::Text t(font, sf::String::fromUtf8(str.begin(), str.end()), 24);
            t.setFillColor(sf::Color::White);
            auto pos = btn.getPosition();
            auto size = btn.getSize();
            t.setPosition({ pos.x + (size.x - t.getLocalBounds().size.x) / 2.f,
                            pos.y + (size.y - t.getLocalBounds().size.y) / 2.f - 5.f });
            window.draw(t);
            };

        drawBtnText("Главное меню", pauseMenuBtn);
        drawBtnText("Заново", pauseRestartBtn);
        drawBtnText("Продолжить", pauseContinueBtn);
    }

    window.display();
}

void Game::update(float deltaTime) {
    waveSystem.update(deltaTime, enemies, map.getPath());
    for (auto& enemy : enemies)
        enemy.update(deltaTime);

    for (auto& tower : towers)
        tower.update(deltaTime, enemies, map.getMapOffset());

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