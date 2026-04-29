#include "Game.hpp"
#include "Colors.hpp"
#include "GameData.hpp"
#include "ResourceManager.hpp"
#include "utils/Math.hpp"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>

// Конструктор инициализирует игру и системы
Game::Game(sf::RenderWindow& window, SettingsManager& settings, SaveManager& saveManager, 
           UpgradeManager& upgradeManager, const std::string& levelPath) :
    window(window), settings(settings), saveManager(saveManager), 
    upgradeManager(upgradeManager), base({ 0, 0 }) {

    map.load(levelPath);
    map.centerOnScreen(window.getSize(), 75.f, 120.f);
    money = map.getStartMoney();
    base = Base(map.getBasePos());
    waveSystem.loadWaves(levelPath);

    initOverlays();
    updateViewSizes(window.getSize());
}

// Построение иерархии контейнеров для оверлеев (в стиле подменюшек Menu)
void Game::initOverlays() {
    auto& font = ResourceManager::getFont("main");
    sf::Vector2f winSize = sf::Vector2f(window.getSize());
    sf::Vector2f btnSize(220.f, 60.f);

    // МЕНЮ ПАУЗЫ
    pauseOverlay = std::make_unique<UI::Container>(winSize);
    pauseOverlay->setDirection(UI::Container::Direction::Column);
    pauseOverlay->setContentAlign(UI::Container::ContentAlign::Center);
    pauseOverlay->setItemAlign(UI::Container::ItemAlign::Center);
    pauseOverlay->setGap(10.f);
    pauseOverlay->setDrawOutline(false);
    pauseOverlay->setBackgroundColor(sf::Color(0, 0, 0, 150));
    pauseOverlay->setDrawBackground(true);

    auto pRoot = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, 300.f));
    pRoot->setDirection(UI::Container::Direction::Column);
    pRoot->setContentAlign(UI::Container::ContentAlign::Center);
    pRoot->setItemAlign(UI::Container::ItemAlign::Center);
    pRoot->setGap(10.f);
    pRoot->setDrawOutline(false);
    pauseModalPtr = pRoot.get();

    auto pTitle = std::make_unique<UI::Text>(font, "ПАУЗА", 96, sf::Vector2f(winSize.x * 0.9f, 100.f));
    pTitle->setAlignment(UI::Text::Align::Center);
    pTitle->setColor(Colors::Theme::TextMain);
    pRoot->addChild(std::move(pTitle));

    auto pNav = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, 80.f));
    pNav->setDirection(UI::Container::Direction::Row);
    pNav->setContentAlign(UI::Container::ContentAlign::Center);
    pNav->setGap(20.f);
    pNav->setDrawOutline(false);

    auto exitBtn = std::make_unique<UI::Button>(font, "В МЕНЮ", btnSize);
    exitBtn->setCallback([this]() { endReason = GameEndReason::ReturnToMenu; });
    pNav->addChild(std::move(exitBtn));

    auto restBtn = std::make_unique<UI::Button>(font, "ЗАНОВО", btnSize);
    restBtn->setCallback([this]() { endReason = GameEndReason::Restart; });
    pNav->addChild(std::move(restBtn));

    auto contBtn = std::make_unique<UI::Button>(font, "ПРОДОЛЖИТЬ", btnSize);
    contBtn->setCallback([this]() { state = GameState::Playing; });
    pNav->addChild(std::move(contBtn));

    pRoot->addChild(std::move(pNav));
    pauseOverlay->addChild(std::move(pRoot));

    // ФИНАЛЬНЫЙ ЭКРАН
    endOverlay = std::make_unique<UI::Container>(winSize);
    endOverlay->setDirection(UI::Container::Direction::Column);
    endOverlay->setContentAlign(UI::Container::ContentAlign::Center);
    endOverlay->setItemAlign(UI::Container::ItemAlign::Center);
    endOverlay->setGap(10.f);
    endOverlay->setDrawOutline(false);
    endOverlay->setBackgroundColor(sf::Color(0, 0, 0, 200));
    endOverlay->setDrawBackground(true);

    auto eRoot = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, 300.f));
    eRoot->setDirection(UI::Container::Direction::Column);
    eRoot->setContentAlign(UI::Container::ContentAlign::Center);
    eRoot->setItemAlign(UI::Container::ItemAlign::Center);
    eRoot->setGap(10.f);
    eRoot->setDrawOutline(false);
    endModalPtr = eRoot.get();

    auto eTitle = std::make_unique<UI::Text>(font, "ФИНАЛ", 60, sf::Vector2f(winSize.x * 0.9f, 60.f));
    eTitle->setAlignment(UI::Text::Align::Center);
    endTitlePtr = eTitle.get();
    eRoot->addChild(std::move(eTitle));

    auto eSub = std::make_unique<UI::Text>(font, "Результат уровня", 32, sf::Vector2f(winSize.x * 0.9f, 60.f));
    eSub->setAlignment(UI::Text::Align::Center);
    endSubTitlePtr = eSub.get();
    eRoot->addChild(std::move(eSub));

    auto eNav = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, 80.f));
    eNav->setDirection(UI::Container::Direction::Row);
    eNav->setContentAlign(UI::Container::ContentAlign::Center);
    eNav->setGap(20.f);
    eNav->setDrawOutline(false);

    auto eExitBtn = std::make_unique<UI::Button>(font, "В МЕНЮ", btnSize);
    eExitBtn->setCallback([this]() { endReason = GameEndReason::ReturnToMenu; });
    eNav->addChild(std::move(eExitBtn));

    auto eRestBtn = std::make_unique<UI::Button>(font, "ЗАНОВО", btnSize);
    eRestBtn->setCallback([this]() { endReason = GameEndReason::Restart; });
    eNav->addChild(std::move(eRestBtn));

    eRoot->addChild(std::move(eNav));
    endOverlay->addChild(std::move(eRoot));
}

// Обновляет камеры и положение контейнеров интерфейса
void Game::updateViewSizes(sf::Vector2u windowSize) {
    float sw = static_cast<float>(windowSize.x);
    float sh = static_cast<float>(windowSize.y);
    float baseScale = sh / 1080.f;

    uiScale = baseScale * settings.get<float>("ui_scale", 1.0f);
    if (uiScale <= 0.1f) uiScale = 1.0f;

    const float minVisibleHeight = 80.f;
    const float maxVisibleHeight = 1200.f;
    
    minZoom = minVisibleHeight / sh;
    maxZoom = maxVisibleHeight / sh;
    
    if (sh > 1440.f) {
        float scale = sh / 1440.f;
        maxZoom *= (1.0f + 0.2f * (scale - 1.0f));
    }
    
    if (minZoom > 0.5f) minZoom = 0.5f;
    if (minZoom < 0.35f) minZoom = 0.35f;
    if (maxZoom < 1.0f) maxZoom = 1.0f;
    if (maxZoom > 2.5f) maxZoom = 2.5f;

    float uiH = sh / uiScale;
    float uiW = uiH * (sw / sh);
    sf::Vector2f logicalSize(uiW, uiH);

    uiView = sf::View(sf::Vector2f(uiW / 2.f, uiH / 2.f), logicalSize);
    worldView = sf::View(sf::Vector2f(sw / 2.f, sh / 2.f), sf::Vector2f(sw, sh));
    
    if (currentZoom < minZoom) currentZoom = minZoom;
    if (currentZoom > maxZoom) currentZoom = maxZoom;

    worldView.zoom(currentZoom);
    
    hud.updateLayout(logicalSize, uiScale * 0.85f);
    hud.setUiScale(uiScale * 0.85f);

    auto updateOverlay = [&](std::unique_ptr<UI::Container>& overlay) {
        if (!overlay) return;
        overlay->setSize(logicalSize);
        overlay->setPosition({ 0, 0 });
        float rootW = logicalSize.x * 0.9f;

        for (size_t i = 0; i < overlay->getChildrenCount(); ++i) {
            auto* child = overlay->getChild(i);
            if (!child) continue;
            child->setSize(sf::Vector2f(rootW, child->getSize().y));

            if (auto* asContainer = dynamic_cast<UI::Container*>(child)) {
                for (size_t j = 0; j < asContainer->getChildrenCount(); ++j) {
                    auto* subChild = asContainer->getChild(j);
                    if (subChild) subChild->setSize(sf::Vector2f(rootW, subChild->getSize().y));
                }
            }
        }
        overlay->rebuild();
    };
    if (endTitlePtr) endTitlePtr->setMaxWidth(logicalSize.x * 0.9f);
    if (endSubTitlePtr) endSubTitlePtr->setMaxWidth(logicalSize.x * 0.9f);

    updateOverlay(pauseOverlay);
    updateOverlay(endOverlay);
}

// Запускает основной цикл игровой сессии
void Game::run() {
    while (window.isOpen() && endReason == GameEndReason::None) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f;

        handleEvents();

        if (state == GameState::Playing) {
            update(dt);
        }
        render();
    }
}

// Обработка всех событий ввода и системных команд
void Game::handleEvents() {
    while (std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) window.close();

        if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            updateViewSizes({ resized->size.x, resized->size.y });
            map.centerOnScreen({ resized->size.x, resized->size.y }, 75.f, 120.f);
            clampView();
        }

        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Space) waveSystem.startWave();
            if (key->code == sf::Keyboard::Key::Escape || key->code == sf::Keyboard::Key::P) {
                if (state == GameState::Playing) state = GameState::Paused;
                else if (state == GameState::Paused) state = GameState::Playing;
            }
        }

        bool uiConsumed = hud.handleEvent(*event, window, uiView);
        if (hud.isPauseRequested()) state = GameState::Paused;
        if (hud.isSkipRequested()) waveSystem.startWave();

        // Обработка продажи выбранной башни
        if (hud.isSellRequested()) {
            Tile* selected = map.getSelectedTile();
            if (selected) {
                auto it = std::find_if(towers.begin(), towers.end(), [&](const Tower& t) {
                    return t.getGridPos() == selected->gridPos;
                });

                if (it != towers.end()) {
                    // Возвращаем 100% только если ПЕРВАЯ волна еще не запущена (состояние Idle)
                    float refundPercent = (waveSystem.getState() == WaveState::Idle) ? 1.0f : 0.65f;
                    money += static_cast<int>(it->getTotalValue() * refundPercent);
                    towers.erase(it);
                    map.setSelectedTile({ -1000.f, -1000.f });
                    hud.hideTowerControls();
                }
            }
            hud.resetRequests();
        }

        // Обработка улучшения выбранной башни
        if (hud.isUpgradeRequested()) {
            Tile* selected = map.getSelectedTile();
            if (selected) {
                auto it = std::find_if(towers.begin(), towers.end(), [&](Tower& t) {
                    return t.getGridPos() == selected->gridPos;
                });

                if (it != towers.end() && it->canUpgradeInGame()) {
                    int cost = it->getInGameUpgradeCost();
                    if (money >= cost) {
                        money -= cost;
                        it->upgradeInGame(cost);
                    }
                }
            }
            hud.resetRequests();
        }

        if (state == GameState::Paused && pauseOverlay) {
            pauseOverlay->handleEvent(*event, window, uiView);
        }
        else if ((state == GameState::GameOver || state == GameState::Victory) && endOverlay) {
            endOverlay->handleEvent(*event, window, uiView);
        }
        else if (state == GameState::Playing && !uiConsumed) {
            // Управление камерой (зум и панорамирование)
            if (const auto* scroll = event->getIf<sf::Event::MouseWheelScrolled>()) {
                float factor = (scroll->delta > 0) ? 0.9f : 1.1f;
                if ((currentZoom * factor >= minZoom) && (currentZoom * factor <= maxZoom)) {
                    sf::Vector2f before = window.mapPixelToCoords(scroll->position, worldView);
                    worldView.zoom(factor);
                    currentZoom *= factor;
                    worldView.move(before - window.mapPixelToCoords(scroll->position, worldView));
                    clampView();
                }
            }
            // ... (остальной ввод без изменений) ...
            if (const auto* touch = event->getIf<sf::Event::TouchBegan>()) {
                if (touch->finger == 0) { isPanning = true; hasMoved = false; startTouchPos = touch->position; lastInputPos = touch->position; }
                if (touch->finger == 1 && sf::Touch::isDown(0)) {
                    isPinching = true; isPanning = false;
                    sf::Vector2f p0(sf::Touch::getPosition(0, window)); sf::Vector2f p1(sf::Touch::getPosition(1, window));
                    initialPinchDistance = std::sqrt(std::pow(p0.x - p1.x, 2) + std::pow(p0.y - p1.y, 2));
                }
            }
            if (const auto* t = event->getIf<sf::Event::TouchMoved>()) {
                sf::Vector2i currentPos = t->position;
                if (isPanning) {
                    float d = std::sqrt(std::pow(currentPos.x - startTouchPos.x, 2) + std::pow(currentPos.y - startTouchPos.y, 2));
                    if (d > 10.f) hasMoved = true;
                    if (!isPinching) {
                        sf::Vector2f delta = sf::Vector2f(lastInputPos - currentPos);
                        worldView.move(delta * (currentZoom * settings.get<float>("sensitivity")));
                        lastInputPos = currentPos;
                        clampView();
                    }
                }
                if (isPinching && sf::Touch::isDown(0) && sf::Touch::isDown(1)) {
                    sf::Vector2i p0 = sf::Touch::getPosition(0, window); sf::Vector2i p1 = sf::Touch::getPosition(1, window);
                    sf::Vector2i mid((p0.x + p1.x) / 2, (p0.y + p1.y) / 2);
                    sf::Vector2f worldBefore = window.mapPixelToCoords(mid, worldView);
                    float newDist = std::sqrt(std::pow((float)p0.x - p1.x, 2) + std::pow((float)p0.y - p1.y, 2));
                    if (std::abs(newDist - initialPinchDistance) > 2.f) {
                        float f = initialPinchDistance / newDist;
                        if (currentZoom * f >= minZoom && currentZoom * f <= maxZoom) {
                            worldView.zoom(f); currentZoom *= f;
                            worldView.move(worldBefore - window.mapPixelToCoords(mid, worldView));
                            clampView();
                        }
                        initialPinchDistance = newDist;
                    }
                }
            }
            sf::Vector2i actionPos(-1, -1);
            if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (click->button == sf::Mouse::Button::Left) actionPos = click->position;
                if (click->button == sf::Mouse::Button::Right) { isPanning = true; lastInputPos = click->position; }
            }
            if (const auto* touchEnd = event->getIf<sf::Event::TouchEnded>()) {
                if (touchEnd->finger == 0 && !hasMoved && !isPinching) actionPos = touchEnd->position;
                if (touchEnd->finger == 0) isPanning = false; if (touchEnd->finger == 1) isPinching = false;
            }
            if (const auto* released = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (released->button == sf::Mouse::Button::Right) isPanning = false;
            }
            if (actionPos.x != -1) processInput(actionPos);
        }
        if (const auto* m = event->getIf<sf::Event::MouseMoved>()) {
            if (isPanning) {
                sf::Vector2f delta = sf::Vector2f(lastInputPos - m->position);
                worldView.move(delta * (currentZoom * settings.get<float>("sensitivity")));
                lastInputPos = m->position;
                clampView();
            }
        }
    }
}

// Обновление всей игровой логики
void Game::update(float deltaTime) {
    if (state != GameState::Playing) return;

    float scaledDt = deltaTime * hud.getGameSpeed();
    map.update(scaledDt);
    waveSystem.update(scaledDt, enemies, map.getPath());

    for (auto& e : enemies) e->update(scaledDt);
    for (auto& t : towers) t.update(scaledDt, enemies, projectiles, map.getMapOffset());
    for (auto& p : projectiles) p.update(scaledDt, enemies);

    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(), [](const Projectile& p) { return !p.isAlive(); }), projectiles.end());

    for (auto& e : enemies) {
        if (e->hasReachedBase()) base.takeDamage(1);
        if (e->isKilled()) {
            money += e->getReward();
            accumulatedGlobalMoney += upgradeManager.getRandomMoney(saveManager.getMoneyMultiplier());
        }
    }

    enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](const std::unique_ptr<Enemy>& e) { return !e->isAlive(); }), enemies.end());

    if (base.isDestroyed()) {
        state = GameState::GameOver;
        if (endTitlePtr) { endTitlePtr->setText("ПОРАЖЕНИЕ"); endTitlePtr->setColor(sf::Color::Red); }
        if (endSubTitlePtr) endSubTitlePtr->setText("Ваша база уничтожена");
    }
    else if (waveSystem.isFinished() && enemies.empty()) {
        state = GameState::Victory;
        saveManager.addMoney(accumulatedGlobalMoney);
        saveManager.save();
        if (endTitlePtr) { endTitlePtr->setText("ПОБЕДА!"); endTitlePtr->setColor(Colors::Theme::TextMain); }
        if (endSubTitlePtr) endSubTitlePtr->setText("Все волны отражены");
    }
}

// Отрисовка всех слоев игры
void Game::render() {
    window.clear(Colors::Theme::Background);
    window.setView(worldView);
    bool slotSelected = hud.getSelectedSlot() != -1;
    map.render(window, !slotSelected);

    Tile* selected = map.getSelectedTile();
    for (auto& t : towers) {
        bool showR = (selected && selected->gridPos == t.getGridPos() && !slotSelected);
        t.render(window, map.getMapOffset(), showR);
    }
    for (auto& e : enemies) e->render(window, map.getMapOffset());
    for (auto& p : projectiles) p.render(window, map.getMapOffset());

    if (selected && selected->type == TileType::Platform && hud.getSelectedSlot() == -1) {
        auto it = std::find_if(towers.begin(), towers.end(), [&](const Tower& t) {
            return t.getGridPos() == selected->gridPos;
        });

        if (it != towers.end()) {
            sf::Vector2f worldCenter = sf::Vector2f(selected->gridPos * 64) + map.getMapOffset() + sf::Vector2f(32.f, 32.f);
            sf::Vector2i pixelPos = window.mapCoordsToPixel(worldCenter, worldView);
            sf::Vector2f uiPos = window.mapPixelToCoords(pixelPos, uiView);
            
            float refundPercent = (waveSystem.getState() == WaveState::Idle) ? 1.0f : 0.65f;
            int sellPrice = (int)(it->getTotalValue() * refundPercent);

            hud.showTowerControls(uiPos, sellPrice, it->getInGameUpgradeCost(), it->canUpgradeInGame(), currentZoom);
        } else {
            hud.hideTowerControls();
        }
    } else {
        hud.hideTowerControls();
    }

    window.setView(uiView);
    hud.render(window, money, base.getLives(), waveSystem.getCurrentWave(), waveSystem.getState());

    if (state == GameState::Paused && pauseOverlay) pauseOverlay->render(window);
    else if ((state == GameState::GameOver || state == GameState::Victory) && endOverlay) endOverlay->render(window);

    window.display();
}

// Ограничение камеры
void Game::clampView() {
    sf::Vector2f center = worldView.getCenter();
    sf::Vector2f size = worldView.getSize();
    float mapLeft = map.getMapOffset().x;
    float mapTop = map.getMapOffset().y;
    float mapRight = mapLeft + map.getWidth() * 64.f;
    float mapBottom = mapTop + map.getHeight() * 64.f;
    float mx = size.x * 0.1f, my = size.y * 0.1f;
    if (center.x < mapLeft - mx) center.x = mapLeft - mx;
    if (center.x > mapRight + mx) center.x = mapRight + mx;
    if (center.y < mapTop - my) center.y = mapTop - my;
    if (center.y > mapBottom + my) center.y = mapBottom + my;
    worldView.setCenter(center);
}

// Обработка клика в мире
void Game::processInput(sf::Vector2i pixelPos) {
    if (isPanning) return;
    sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, worldView);

    if (state == GameState::Playing) {
        Tile* tile = map.getTileAtScreen(worldPos);
        hud.hideTowerControls();

        if (tile && tile->type == TileType::Platform) {
            int slot = hud.getSelectedSlot();
            if (slot != -1) map.setSelectedTile(sf::Vector2f(-1000.f, -1000.f));
            
            if (slot != -1) {
                auto names = GameData::getTowerNames();
                if (slot < (int)names.size()) {
                    std::string name = names[slot];
                    int cost = GameData::getTower(name).cost;
                    bool occupied = false;
                    for (auto& t : towers) if (t.getGridPos() == tile->gridPos) { occupied = true; break; }

                    if (!occupied && money >= cost) {
                        money -= cost;
                        towers.emplace_back(name, tile->gridPos, upgradeManager);
                        hud.resetSelectedSlot();
                    }
                }
            } 
            else {
                map.setSelectedTile(worldPos);
                auto it = std::find_if(towers.begin(), towers.end(), [&](const Tower& t) {
                    return t.getGridPos() == tile->gridPos;
                });

                if (it != towers.end()) {
                    sf::Vector2f tileWorldCenter = sf::Vector2f(tile->gridPos * 64) + map.getMapOffset() + sf::Vector2f(32.f, 32.f);
                    sf::Vector2i pixelCoords = window.mapCoordsToPixel(tileWorldCenter, worldView);
                    sf::Vector2f uiPos = window.mapPixelToCoords(pixelCoords, uiView);
                    float refundPercent = (waveSystem.getState() == WaveState::Idle) ? 1.0f : 0.65f;
                    hud.showTowerControls(uiPos, (int)(it->getTotalValue() * refundPercent), it->getInGameUpgradeCost(), it->canUpgradeInGame());
                }
            }
        } else {
            map.setSelectedTile(worldPos);
        }
    }
}

GameEndReason Game::getEndReason() const { return endReason; }

void Game::cleanup() {
    if (pauseOverlay) pauseOverlay.reset();
    if (endOverlay) endOverlay.reset();
    pauseModalPtr = nullptr; endModalPtr = nullptr; endTitlePtr = nullptr; endSubTitlePtr = nullptr;
}
