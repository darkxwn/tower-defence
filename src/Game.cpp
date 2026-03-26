#include "Colors.hpp"
#include "Game.hpp"
#include "GameData.hpp"
#include "ResourceManager.hpp"
#include "WaveSystem.hpp"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <iostream>

Game::Game(sf::RenderWindow& window, const std::string& levelPath)
    : window(window), base(sf::Vector2i{ 0, 0 })
{
    // Загружаем карту указанного уровня
    map.load(levelPath);
    map.centerOnScreen(window.getSize(), 75.f, 120.f);
    money = map.getStartMoney();

    // Пересоздаём Base с правильной позицией после загрузки карты
    base = Base(map.getBasePos());

    // Загружаем волны из того же .map файла
    waveSystem.loadWaves(levelPath);
}

void Game::run() {
    while (window.isOpen() && !returnToMenu) {
        float deltaTime = clock.restart().asSeconds();
        handleEvents();
        if (gameState == GameState::Playing)
            update(deltaTime);
        render();
    }
}

bool Game::shouldReturnToMenu() const { return returnToMenu; }

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
                    gameState = GameState::Paused;
            }

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

            // Кнопка паузы в HUD
            if (hud.isPauseClicked()) {
                if (gameState == GameState::Playing)
                    gameState = GameState::Paused;
                else if (gameState == GameState::Paused)
                    gameState = GameState::Playing;
            }

            // Кнопки оверлея паузы
            if (gameState == GameState::Paused && click->button == sf::Mouse::Button::Left) {
                if (pauseContinueBtn.getGlobalBounds().contains(mousePos)) {
                    gameState = GameState::Playing;
                }
                if (pauseRestartBtn.getGlobalBounds().contains(mousePos)) {
                    // Перезапуск — возврат в меню (пока без авто-рестарта)
                    returnToMenu = true;
                }
                if (pauseMenuBtn.getGlobalBounds().contains(mousePos)) {
                    returnToMenu = true;
                }
            }

            // Кнопка скипа волны
            if (hud.isSkipClicked())
                waveSystem.startWave();

            if (click->button == sf::Mouse::Button::Left) {
                Tile* tile = map.getTileAtScreen(mousePos);
                if (tile && tile->type == TileType::Platform) {
                    int slot = hud.getSelectedSlot();
                    if (slot != -1) {
                        auto towerNames = GameData::getTowerNames();
                        if (slot < (int)towerNames.size()) {
                            std::string name = towerNames[slot];
                            int cost = GameData::getTower(name).cost;

                            bool occupied = false;
                            for (auto& t : towers)
                                if (t.getGridPos() == tile->gridPos) { occupied = true; break; }

                            if (!occupied && money >= cost) {
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

    // Оверлей паузы
    if (gameState == GameState::Paused) {
        auto& font = ResourceManager::getFont("main");
        auto winSize = window.getSize();
        float cx = winSize.x / 2.f;
        float cy = winSize.y / 2.f;

        // Затемнение экрана
        sf::RectangleShape overlay;
        overlay.setSize(sf::Vector2f(winSize));
        overlay.setFillColor(sf::Color(0, 0, 0, 150));
        window.draw(overlay);

        // Текст "ПАУЗА"
        std::string pauseStr = "ПАУЗА";
        sf::Text pauseText(font, sf::String::fromUtf8(pauseStr.begin(), pauseStr.end()), 128);
        pauseText.setFillColor(sf::Color::White);
        pauseText.setPosition({ cx - pauseText.getLocalBounds().size.x / 2.f, cy - 180.f });
        window.draw(pauseText);

        // Размеры и расположение кнопок
        float btnW = 220.f, btnH = 60.f, btnGap = 30.f;
        float totalW = btnW * 3 + btnGap * 2;
        float btnY = cy - btnH / 2.f;

        // Кнопка "Главное меню"
        pauseMenuBtn.setSize({ btnW, btnH });
        pauseMenuBtn.setFillColor(Colors::slotBg);
        pauseMenuBtn.setPosition({ cx - totalW / 2.f, btnY });
        window.draw(pauseMenuBtn);

        // Кнопка "Заново"
        pauseRestartBtn.setSize({ btnW, btnH });
        pauseRestartBtn.setFillColor(Colors::slotBg);
        pauseRestartBtn.setPosition({ cx - btnW / 2.f, btnY });
        window.draw(pauseRestartBtn);

        // Кнопка "Продолжить"
        pauseContinueBtn.setSize({ btnW, btnH });
        pauseContinueBtn.setFillColor(Colors::slotBg);
        pauseContinueBtn.setPosition({ cx + totalW / 2.f - btnW, btnY });
        window.draw(pauseContinueBtn);

        // Вспомогательная лямбда для текста на кнопке
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
        drawBtnText("Заново",       pauseRestartBtn);
        drawBtnText("Продолжить",   pauseContinueBtn);
    }

    window.display();
}

void Game::update(float deltaTime) {
    waveSystem.update(deltaTime, enemies, map.getPath());

    for (auto& enemy : enemies)
        enemy.update(deltaTime);

    for (auto& tower : towers)
        tower.update(deltaTime, enemies, map.getMapOffset());

    // Враги, дошедшие до базы, наносят урон
    for (auto& enemy : enemies)
        if (enemy.hasReachedBase())
            base.takeDamage(1);

    // Награда за убитых врагов
    for (auto& enemy : enemies)
        if (enemy.isKilled())
            money += GameData::getEnemy(enemy.getType()).reward;

    // Удаляем мёртвых врагов
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [](const Enemy& e) { return !e.isAlive(); }),
        enemies.end()
    );

    // Проверка поражения
    if (base.isDestroyed())
        gameState = GameState::GameOver;

    // Проверка победы
    if (waveSystem.isFinished() && enemies.empty())
        gameState = GameState::Victory;
}
