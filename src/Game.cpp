#include "Game.hpp"
#include "Colors.hpp"
#include "GameData.hpp"
#include "ResourceManager.hpp"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>

// Конструктор инициализирует игру и UI элементы
Game::Game(sf::RenderWindow& window, SettingsManager& settings, const std::string& levelPath) :
    window(window),
    settings(settings),
    lblPause(ResourceManager::getFont("main"), "", 128),
    lblEndScreen(ResourceManager::getFont("main"), "", 112),
    subLblEndScreen(ResourceManager::getFont("main"), "", 32),
    base({ 0, 0 }) {

    auto& font = ResourceManager::getFont("main");
    sf::Vector2f bSize(220.f, 60.f);

    // Создание кнопок паузы
    pauseButtons.reserve(3);
    { UI::Button b(font, "Завершить", bSize); b.setCallback([this]() { endReason = GameEndReason::ReturnToMenu; }); pauseButtons.push_back(std::move(b)); }
    { UI::Button b(font, "Заново", bSize); b.setCallback([this]() { endReason = GameEndReason::Restart;      }); pauseButtons.push_back(std::move(b)); }
    { UI::Button b(font, "Продолжить", bSize); b.setCallback([this]() { state = GameState::Playing;              }); pauseButtons.push_back(std::move(b)); }

    // Создание кнопок финального экрана
    endScreenButtons.reserve(2);
    { UI::Button b(font, "В меню", bSize); b.setCallback([this]() { endReason = GameEndReason::ReturnToMenu; }); endScreenButtons.push_back(std::move(b)); }
    { UI::Button b(font, "Заново", bSize); b.setCallback([this]() { endReason = GameEndReason::Restart;      }); endScreenButtons.push_back(std::move(b)); }

    map.load(levelPath);
    map.centerOnScreen(window.getSize(), 75.f, 120.f);
    money = map.getStartMoney();
    base = Base(map.getBasePos());
    waveSystem.loadWaves(levelPath);

    updateViewSizes(window.getSize());
}

//// Обновляет размеры камер и позиции элементов интерфейса
//void Game::updateViewSizes(sf::Vector2u windowSize) {
//    float sw = static_cast<float>(windowSize.x); // физическая ширина
//    float sh = static_cast<float>(windowSize.y); // физическая высота
//    float aspect = sw / sh;                      // соотношение сторон
//
//    uiScale = settings.getFloat("ui_scale");     // чтение масштаба из настроек
//    float uiH = sh / uiScale;                    // логическая высота UI
//    float uiW = uiH * aspect;                    // логическая ширина UI
//    uiView = sf::View(sf::FloatRect({ 0.f, 0.f }, { uiW, uiH }));
//    //uiView = sf::View({ 0.f, 0.f }, { uiW, uiH });
//    worldView = sf::View({ 0.f, 0.f }, { sw, sh });
//    worldView.zoom(currentZoom);
//    worldView.setSize({ sw / uiScale, sh / uiScale });
//
//    hud.updateLayout(uiView.getSize());
//}

// Обновляет камеры и позиции оверлеев
void Game::updateViewSizes(sf::Vector2u windowSize) {
    float sw = static_cast<float>(windowSize.x);
    float sh = static_cast<float>(windowSize.y);
    uiScale = settings.getFloat("ui_scale");

    float uiH = 1080.f / uiScale;
    float uiW = uiH * (sw / sh);

    //uiView = sf::View(sf::FloatRect({ 0.f, 0.f }, { uiW, uiH }));
    //worldView = sf::View(sf::FloatRect({ 0.f, 0.f }, { sw, sh }));
    uiView = sf::View(sf::Vector2f(uiW / 2.f, uiH / 2.f), sf::Vector2f(uiW, uiH));
    worldView = sf::View(sf::Vector2f(sw / 2.f, sh / 2.f), sf::Vector2f(sw, sh));
    worldView.zoom(currentZoom);

    hud.updateLayout(uiView.getSize());

    // Позиционирование кнопок оверлеев
    sf::Vector2f ws = uiView.getSize();
    float cx = ws.x / 2.f;
    float cy = ws.y / 2.f;

    if (pauseButtons.size() >= 3) {
        float btnW = 220.f, gap = 30.f;
        float totalW = btnW * 3 + gap * 2;
        float btnY = cy + 60.f;
        pauseButtons[0].setPosition({ cx - totalW / 2.f,           btnY });
        pauseButtons[1].setPosition({ cx - btnW / 2.f,             btnY });
        pauseButtons[2].setPosition({ cx + totalW / 2.f - btnW,    btnY });
    }

    if (endScreenButtons.size() >= 2) {
        float btnW = 220.f, gap = 24.f;
        float totalW = btnW * 2 + gap;
        endScreenButtons[0].setPosition({ cx - totalW / 2.f,           cy + 40.f });
        endScreenButtons[1].setPosition({ cx - totalW / 2.f + btnW + gap, cy + 40.f });
    }
}


// Запускает основной цикл игровой сессии
void Game::run() {
    while (window.isOpen() && endReason == GameEndReason::None) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f; // ограничение дельты при лагах

        handleEvents();

        if (state == GameState::Playing) {
            update(dt);
        }
        render();
    }
}

// Расчитывает геометрию кнопок паузы (кнопки уже созданы в конструкторе)
Game::PauseLayout Game::computePauseBtnLayout() {
    return PauseLayout{};
}

// Расчитывает геометрию кнопок финального экрана (кнопки уже созданы в конструкторе)
Game::EndScreenLayout Game::computeEndScreenLayout() {
    return EndScreenLayout{};
}

// Обрабатывает системные события, ввод и навигацию
void Game::handleEvents() {
    while (std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) window.close();

        // 1. Служебные события
        if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            updateViewSizes({ resized->size.x, resized->size.y });
            map.centerOnScreen({ resized->size.x, resized->size.y }, 75.f, 120.f);
            clampView();
        }

        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Space) waveSystem.startWave();
            if (key->code == sf::Keyboard::Key::Escape || key->code == sf::Keyboard::Key::P) {
                state = (state == GameState::Playing) ? GameState::Paused : GameState::Playing;
            }
        }

        // 2. Проброс событий в UI
        hud.handleEvent(*event, window, uiView);
        if (hud.isPauseRequested()) state = GameState::Paused;
        if (hud.isSkipRequested())  waveSystem.startWave();

        // Обработка оверлеев
        if (state == GameState::Paused) {
            for (auto& btn : pauseButtons) btn.handleEvent(*event, window, uiView);
        }
        if (state == GameState::Victory || state == GameState::GameOver) {
            for (auto& btn : endScreenButtons) btn.handleEvent(*event, window, uiView);
        }

        // 3. Навигация по карте (только в процессе игры)
        if (state == GameState::Playing) {
            // Зум колесиком
            if (const auto* scroll = event->getIf<sf::Event::MouseWheelScrolled>()) {
                float factor = (scroll->delta > 0) ? 0.9f : 1.1f;
                if ((currentZoom * factor >= 0.5f) && (currentZoom * factor <= 1.6f)) {
                    sf::Vector2f before = window.mapPixelToCoords(scroll->position, worldView);
                    worldView.zoom(factor);
                    currentZoom *= factor;
                    worldView.move(before - window.mapPixelToCoords(scroll->position, worldView));
                    clampView();
                }
            }

            // Тач-навигация и Pinch-zoom
            if (const auto* touch = event->getIf<sf::Event::TouchBegan>()) {
                if (touch->finger == 0) {
                    isPanning = true;
                    hasMoved = false;
                    startTouchPos = touch->position;
                    lastInputPos = touch->position;
                }
                if (touch->finger == 1 && sf::Touch::isDown(0)) {
                    isPinching = true;
                    isPanning = false;
                    sf::Vector2f p0(sf::Touch::getPosition(0, window));
                    sf::Vector2f p1(sf::Touch::getPosition(1, window));
                    initialPinchDistance = std::sqrt(std::pow(p0.x - p1.x, 2) + std::pow(p0.y - p1.y, 2));
                }
            }

            // Перемещение тачем
            if (const auto* t = event->getIf<sf::Event::TouchMoved>()) {
                sf::Vector2i currentPos = t->position;
                if (isPanning) {
                    float d = std::sqrt(std::pow(currentPos.x - startTouchPos.x, 2) + std::pow(currentPos.y - startTouchPos.y, 2));
                    if (d > 10.f) hasMoved = true;
                    if (!isPinching) {
                        sf::Vector2f delta = sf::Vector2f(lastInputPos - currentPos);
                        worldView.move(delta * (currentZoom * settings.getFloat("sensitivity")));
                        lastInputPos = currentPos;
                        clampView();
                    }
                }
            }

            // Логика Pinch-Zoom
            if (isPinching && event->is<sf::Event::TouchMoved>() && sf::Touch::isDown(0) && sf::Touch::isDown(1)) {
                sf::Vector2i p0 = sf::Touch::getPosition(0, window);
                sf::Vector2i p1 = sf::Touch::getPosition(1, window);
                sf::Vector2f worldBefore = window.mapPixelToCoords((p0 + p1) / 2, worldView);
                float newDist = std::sqrt(std::pow((float)p0.x - p1.x, 2) + std::pow((float)p0.y - p1.y, 2));

                if (std::abs(newDist - initialPinchDistance) > 2.f) {
                    float f = initialPinchDistance / newDist;
                    if (currentZoom * f > 0.5f && currentZoom * f < 1.6f) {
                        worldView.zoom(f);
                        currentZoom *= f;
                        worldView.move(worldBefore - window.mapPixelToCoords((p0 + p1) / 2, worldView));
                        clampView();
                    }
                    initialPinchDistance = newDist;
                }
            }
        }

        // Панорамирование мышью через ПКМ (в любом состоянии игры)
        if (const auto* m = event->getIf<sf::Event::MouseMoved>()) {
            if (isPanning) {
                sf::Vector2f delta = sf::Vector2f(lastInputPos - m->position);
                worldView.move(delta * (currentZoom * settings.getFloat("sensitivity")));
                lastInputPos = m->position;
                clampView();
            }
        }

        // 4. Клики и действия
        sf::Vector2i actionPos(-1, -1);
        if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (click->button == sf::Mouse::Button::Left) actionPos = click->position;
            if (click->button == sf::Mouse::Button::Right) {
                isPanning = true;
                lastInputPos = click->position;
            }
        }
        if (const auto* touch = event->getIf<sf::Event::TouchEnded>()) {
            if (touch->finger == 0 && !hasMoved && !isPinching) actionPos = touch->position;
            if (touch->finger == 0) isPanning = false;
            if (touch->finger == 1) isPinching = false;
        }
        if (const auto* released = event->getIf<sf::Event::MouseButtonReleased>()) {
            if (released->button == sf::Mouse::Button::Right) isPanning = false;
        }

        if (actionPos.x != -1) processInput(actionPos);
    }
}

// Логика взаимодействия игрока с миром (постройка башен)
void Game::processInput(sf::Vector2i pixelPos) {
    if (isPanning) return;

    sf::Vector2f uiPos = window.mapPixelToCoords(pixelPos, uiView);
    sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, worldView);

    if (state == GameState::Paused) {
        // логика кнопок паузы обрабатывается в handleEvents через UI::Button
        return;
    }

    if (state == GameState::Playing) {
        Tile* tile = map.getTileAtScreen(worldPos);
        if (tile && tile->type == TileType::Platform) {
            int slot = hud.getSelectedSlot();
            if (slot != -1) {
                auto names = GameData::getTowerNames();
                if (slot < (int)names.size()) {
                    std::string name = names[slot];
                    int cost = GameData::getTower(name).cost;
                    bool occupied = false;
                    for (auto& t : towers) if (t.getGridPos() == tile->gridPos) { occupied = true; break; }

                    if (!occupied && money >= cost) {
                        money -= cost;
                        towers.emplace_back(name, tile->gridPos);
                        hud.resetSelectedSlot();
                    }
                }
            }
            else {
                map.setSelectedTile(worldPos);
            }
        }
        else {
            map.setSelectedTile(worldPos);
        }
    }
}

// Обновление игровой логики
void Game::update(float deltaTime) {
    float scaledDt = deltaTime * hud.getGameSpeed(); // учет ускорения игры
    map.update(scaledDt);
    waveSystem.update(scaledDt, enemies, map.getPath());

    for (auto& e : enemies) e->update(scaledDt);
    for (auto& t : towers) t.update(scaledDt, enemies, projectiles, map.getMapOffset());
    for (auto& p : projectiles) p.update(scaledDt, enemies);

    // очистка мертвых объектов
    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(), [](const Projectile& p) { return !p.isAlive(); }), projectiles.end());

    for (auto& e : enemies) {
        if (e->hasReachedBase()) base.takeDamage(1);
        if (e->isKilled()) money += GameData::getEnemy(e->getType()).reward;
    }

    enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](const std::unique_ptr<Enemy>& e) { return !e->isAlive(); }), enemies.end());

    // проверка условий конца игры
    if (base.isDestroyed()) {
        state = GameState::GameOver;
    }
    else if (waveSystem.isFinished() && enemies.empty()) {
        state = GameState::Victory;
    }
}

// Отрисовка кадра
void Game::render() {
    window.clear(Colors::Palette::Gray90);

    // 1. Отрисовка игрового мира
    window.setView(worldView);
    map.render(window);

    Tile* selected = map.getSelectedTile();
    for (auto& t : towers) {
        bool showR = (selected && selected->gridPos == t.getGridPos());
        t.render(window, map.getMapOffset(), showR);
    }
    for (auto& e : enemies) e->render(window, map.getMapOffset());
    for (auto& p : projectiles) p.render(window, map.getMapOffset());

    // 2. Отрисовка интерфейса
    window.setView(uiView);
    hud.render(window, money, base.getLives(), waveSystem.getCurrentWave(), waveSystem.getState());

    if (state == GameState::Paused) renderPauseOverlay();
    if (state == GameState::Victory || state == GameState::GameOver) renderEndScreen();

    window.display();
}

// Отрисовка оверлея паузы
void Game::renderPauseOverlay() {
    sf::Vector2f ws = uiView.getSize();
    sf::RectangleShape dim({ ws.x, ws.y });
    dim.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(dim);

    // Текст "ПАУЗА"
    lblPause.setText("ПАУЗА");
    lblPause.setAlignment(UI::Text::Align::Center);
    lblPause.setPosition({ ws.x / 2.f, 180.f });
    lblPause.render(window);

    for (auto& btn : pauseButtons) {
        btn.render(window);
    }
}

// Отрисовка финального экрана (Победа/Поражение)
void Game::renderEndScreen() {
    sf::Vector2f ws = uiView.getSize();
    sf::RectangleShape dim({ ws.x, ws.y });
    dim.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(dim);

    bool isWin = (state == GameState::Victory);
    lblEndScreen.setText(isWin ? "ПОБЕДА!" : "ПОРАЖЕНИЕ");
    lblEndScreen.setColor(isWin ? Colors::Theme::TextMoney : Colors::Theme::TextLives);
    lblEndScreen.setPosition({ ws.x / 2.f - lblEndScreen.getLocalBounds().size.x / 2.f, 200.f });
    lblEndScreen.render(window);

    subLblEndScreen.setText(isWin ? "Все волны отражены!" : "База уничтожена");
    subLblEndScreen.setPosition({ ws.x / 2.f - subLblEndScreen.getLocalBounds().size.x / 2.f, ws.y / 2.f - 70.f });
    subLblEndScreen.render(window);

    for (auto& btn : endScreenButtons) btn.render(window);
}

// Удержание камеры в пределах карты
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

GameEndReason Game::getEndReason() const { return endReason; }