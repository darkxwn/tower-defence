#include "Colors.hpp"
#include "Game.hpp"
#include "GameData.hpp"
#include "ResourceManager.hpp"
#include <SFML/Graphics.hpp>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
//  Конструктор
// ─────────────────────────────────────────────────────────────────────────────
Game::Game(sf::RenderWindow& window, const std::string& levelPath)
    : window(window), base(sf::Vector2i{ 0, 0 })
{
    // Инициализируем камеры
    updateViewSizes(window.getSize());

    map.load(levelPath);
    map.centerOnScreen(window.getSize(), 75.f, 120.f);
    money = map.getStartMoney();
    base = Base(map.getBasePos());
    waveSystem.loadWaves(levelPath);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Обновление камер (инициализация, ресайз окна)
// ─────────────────────────────────────────────────────────────────────────────
void Game::updateViewSizes(sf::Vector2u windowSize) {
    float sw = static_cast<float>(windowSize.x);
    float sh = static_cast<float>(windowSize.y);
    float aspect = sw / sh;

#ifdef __ANDROID__
    uiScale = 1.6f;
#else
    uiScale = 1.0f; // На ПК оставляем 1 к 1
#endif
    
    float uiH = sh / uiScale;
    float uiW = uiH * aspect;

    uiView = sf::View(sf::FloatRect({ 0.f, 0.f }, { uiW, uiH }));

    worldView = sf::View(sf::FloatRect({ 0.f, 0.f }, { sw, sh }));
    worldView.zoom(currentZoom);
    worldView.setCenter({ sw / 2.f, sh / 2.f });

    worldView.setSize({ sw / uiScale, sh / uiScale });
}

// ─────────────────────────────────────────────────────────────────────────────
//  Главный цикл сессии
// ─────────────────────────────────────────────────────────────────────────────
void Game::run() {
    while (window.isOpen() && endReason == GameEndReason::None) {
        float dt = clock.restart().asSeconds();
        // Ограничиваем дельту: при потере фокуса/лагах враги не прыгают
        if (dt > 0.1f) dt = 0.1f;

        handleEvents();

        if (state == GameState::Playing)
            update(dt);

        render();
    }
}

GameEndReason Game::getEndReason() const { return endReason; }

// ─────────────────────────────────────────────────────────────────────────────
//  Вычисление позиций кнопок паузы
// ─────────────────────────────────────────────────────────────────────────────
void Game::computePauseBtnLayout() {
    sf::Vector2f ws = uiView.getSize();
    float cx    = ws.x / 2.f;
    float cy    = ws.y / 2.f;
    float btnW  = 220.f;
    float btnH  = 60.f;
    float btnGap = 30.f;
    float totalW = btnW * 3 + btnGap * 2;
    // Кнопки размещаются в нижней половине экрана, на расстоянии от текста
    float btnY  = cy + 60.f;

    pauseMenuRect     = sf::FloatRect({cx - totalW / 2.f,              btnY}, {btnW, btnH});
    pauseRestartRect  = sf::FloatRect({cx - btnW / 2.f,                btnY}, {btnW, btnH});
    pauseContinueRect = sf::FloatRect({cx + totalW / 2.f - btnW,       btnY}, {btnW, btnH});
}

//  Обработка событий
void Game::handleEvents() {
    while (std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            window.close();

        // 1. КАСАНИЕ (TouchBegan)
        if (const auto* touch = event->getIf<sf::Event::TouchBegan>()) {
            if (touch->finger == 0) {
                isPanning = true;
                lastInputPos = touch->position;
            }
            // Как только коснулся второй палец — включаем режим зума
            if (touch->finger == 1) {
                isPinching = true;
                isPanning = false; // Блокируем перемещение при зуме

                sf::Vector2f p0 = sf::Vector2f(sf::Touch::getPosition(0, window));
                sf::Vector2f p1 = sf::Vector2f(sf::Touch::getPosition(1, window));
                initialPinchDistance = std::sqrt(std::pow(p0.x - p1.x, 2) + std::pow(p0.y - p1.y, 2));
            }
        }

        // 2. ДВИЖЕНИЕ (TouchMoved)
        if (const auto* touch = event->getIf<sf::Event::TouchMoved>()) {
            if (isPinching && sf::Touch::isDown(0) && sf::Touch::isDown(1)) {
                sf::Vector2i p0_px = sf::Touch::getPosition(0, window);
                sf::Vector2i p1_px = sf::Touch::getPosition(1, window);

                // 1. Находим центр щипка в пикселях
                sf::Vector2i center_px((p0_px.x + p1_px.x) / 2, (p0_px.y + p1_px.y) / 2);

                // 2. Запоминаем, какая точка карты была в этом центре ДО зума
                sf::Vector2f worldPosBefore = window.mapPixelToCoords(center_px, worldView);

                float dx = (float)(p0_px.x - p1_px.x);
                float dy = (float)(p0_px.y - p1_px.y);
                float newDist = std::sqrt(dx * dx + dy * dy);

                if (newDist > 10.f && std::abs(newDist - initialPinchDistance) > 1.0f) {
                    float factor = initialPinchDistance / newDist;

                    float nextZoom = currentZoom * factor;
                    if (nextZoom > 0.4f && nextZoom < 1.3f) {
                        // 3. Выполняем сам зум
                        worldView.zoom(factor);
                        currentZoom = nextZoom;

                        // 4. Находим, где та же точка в пикселях оказалась ПОСЛЕ зума
                        // Для этого снова пересчитываем координаты той же экранной точки
                        sf::Vector2f worldPosAfter = window.mapPixelToCoords(center_px, worldView);

                        // 5. Сдвигаем камеру так, чтобы точка "worldPosBefore" совпала с "worldPosAfter"
                        worldView.move(worldPosBefore - worldPosAfter);

                        clampView();
                    }
                    initialPinchDistance = newDist;
                }
            } else if (isPanning && !isPinching && touch->finger == 0) {
                // Обычное перемещение
                sf::Vector2f delta = sf::Vector2f(lastInputPos - touch->position);
                worldView.move(delta * currentZoom);
                lastInputPos = touch->position;

                clampView();
            }
        }

        // 3. ОТПУСКАНИЕ (TouchEnded)
        if (const auto* touch = event->getIf<sf::Event::TouchEnded>()) {
            if (touch->finger == 0 || touch->finger == 1) {
                isPinching = false;
                isPanning = false;
            }

            // ЛОГИКА КЛИКА (Постройка башни) | если не зум и не сдвиг
            if (touch->finger == 0 && !isPinching) {
                // Если палец почти не двигался, считаем это кликом
                processInput(touch->position);
            }
        }

        // РЕЗАЙЗ
        if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            sf::Vector2u newSize = { resized->size.x, resized->size.y };
            updateViewSizes(newSize);
            map.centerOnScreen(newSize, 75.f, 120.f);
        }

        // ВВОД (МЫШЬ + ТАЧ)
        if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (click->button == sf::Mouse::Button::Left)
                processInput(click->position);
        }
        if (const auto* touch = event->getIf<sf::Event::TouchBegan>()) {
            processInput(touch->position);
        }

        // ПЕРЕМЕЩЕНИЕ (МЫШЬ + ТАЧ)
        if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (click->button == sf::Mouse::Button::Middle || click->button == sf::Mouse::Button::Right) {
                isPanning = true;
                lastInputPos = click->position;
            }
        }

        if (const auto* mouseMove = event->getIf<sf::Event::MouseMoved>()) {
            if (isPanning) {
                sf::Vector2f delta = sf::Vector2f(lastInputPos - mouseMove->position);
                worldView.move(delta * currentZoom);
                lastInputPos = mouseMove->position;

                clampView();
            }
        }

        if (const auto* touch = event->getIf<sf::Event::TouchBegan>()) {
            if (touch->finger == 0) {
                isPanning = true;
                lastInputPos = touch->position;
            }
        }

        if (const auto* touchMove = event->getIf<sf::Event::TouchMoved>()) {
            if (isPanning && !isPinching && touchMove->finger == 0) {
                sf::Vector2f delta = sf::Vector2f(lastInputPos - touchMove->position);
                worldView.move(delta * currentZoom);
                lastInputPos = touchMove->position;

                clampView();
            }
        }

        // Кнопки отпустили
        if (event->is<sf::Event::MouseButtonReleased>() || event->is<sf::Event::TouchEnded>()) {
            isPanning = false;
        }

        // ЗУМ (Колесико мыши)
        if (const auto* scroll = event->getIf<sf::Event::MouseWheelScrolled>()) {
            if (scroll->wheel == sf::Mouse::Wheel::Vertical) {
                float factor = (scroll->delta > 0) ? 0.9f : 1.1f;
                float nextZoom = currentZoom * factor;
                if (nextZoom >= 0.4f && nextZoom <= 1.6f) {
                    // 3. ЗУМ В ТОЧКУ КУРСОРA (Математика как для Android)
                    // Запоминаем, где была мышь в координатах мира ДО зума
                    sf::Vector2f worldPosBefore = window.mapPixelToCoords(scroll->position, worldView);

                    worldView.zoom(factor);
                    currentZoom = nextZoom;
                    sf::Vector2f worldPosAfter = window.mapPixelToCoords(scroll->position, worldView);

                    // Сдвигаем камеру на разницу, чтобы курсор остался над той же точкой карты
                    worldView.move(worldPosBefore - worldPosAfter);

                    clampView();
                }
            }
        }

        // КЛАВИАТУРА
        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Space) waveSystem.startWave();
            if (key->code == sf::Keyboard::Key::Escape) {
                state = (state == GameState::Playing) ? GameState::Paused : GameState::Playing;
            }
        }
    }
}

void Game::processInput(sf::Vector2i pixelPos) {
    // Если мы перемещаем карту - игнорируем постройки и интерфейс
    if (isPanning) return;

    // 1. Сначала проверяем интерфейс (используем uiView)
    sf::Vector2f uiPos = window.mapPixelToCoords(pixelPos, uiView);
    sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, worldView);

    if (state == GameState::Paused) {
        if (pauseContinueRect.contains(uiPos)) state = GameState::Playing;
        else if (pauseRestartRect.contains(uiPos)) endReason = GameEndReason::Restart;
        else if (pauseMenuRect.contains(uiPos)) endReason = GameEndReason::ReturnToMenu;
        return;
    }

    if (state == GameState::Victory || state == GameState::GameOver) {
        if (endMenuRect.contains(uiPos)) endReason = GameEndReason::ReturnToMenu;
        else if (endRestartRect.contains(uiPos)) endReason = GameEndReason::Restart;
        return;
    }

    // Клик по игровому HUD (пауза, слоты)
    hud.handleClick(uiPos);
    if (hud.isPauseClicked()) state = GameState::Paused;
    if (hud.isSkipClicked())  waveSystem.startWave();

    // 2. Если не попали в UI кнопки, проверяем мир (используем worldView)
    if (state == GameState::Playing) {
        Tile* tile = map.getTileAtScreen(worldPos); // Передаем worldPos!
        if (tile && tile->type == TileType::Platform) {
            int slot = hud.getSelectedSlot();
            if (slot != -1) {
                auto names = GameData::getTowerNames();
                if (slot < (int)names.size()) {
                    std::string name = names[slot];
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
                map.setSelectedTile(worldPos);
            }
        } else {
            map.setSelectedTile(worldPos);
        }
    }
}

//  Обновление
void Game::update(float dt) {
    waveSystem.update(dt, enemies, map.getPath());

    for (auto& e : enemies) e.update(dt);
    for (auto& t : towers)  t.update(dt, enemies, map.getMapOffset());

    // Враги у базы — наносят урон
    for (auto& e : enemies)
        if (e.hasReachedBase())
            base.takeDamage(1);

    // Награда за убитых
    for (auto& e : enemies)
        if (e.isKilled())
            money += GameData::getEnemy(e.getType()).reward;

    // Удаляем мёртвых врагов
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [](const Enemy& e){ return !e.isAlive(); }),
        enemies.end());

    // Проверка условий завершения
    if (base.isDestroyed())
        state = GameState::GameOver;
    else if (waveSystem.isFinished() && enemies.empty())
        state = GameState::Victory;
}

// Метод отрисовки
void Game::render() {
    window.clear(Colors::gameBg);

    // СЛОЙ 1: МИР (Карта, Враги, Башни)
    window.setView(worldView);
    map.render(window);

    Tile* selected = map.getSelectedTile();
    for (auto& t : towers) {
        bool showR = selected && selected->gridPos == t.getGridPos();
        t.render(window, map.getMapOffset(), showR);
    }
    for (auto& e : enemies)
        e.render(window, map.getMapOffset());

    // СЛОЙ 2: ИНТЕРФЕЙС (HUD, Оверлеи)
    window.setView(uiView);
    hud.render(window, money, base.getLives(), waveSystem.getCurrentWave(), waveSystem.getState());

    if (state == GameState::Paused)              renderPauseOverlay();
    if (state == GameState::Victory || state == GameState::GameOver) renderEndScreen();

    window.display();
}

// Отрисовка оверлея паузы
void Game::renderPauseOverlay() {
    auto& font = ResourceManager::getFont("main");
    // Используем размер uiView (всегда 1920x1080), а не окна!
    sf::Vector2f uiSize = uiView.getSize();
    float cx = uiSize.x / 2.f;
    float cy = uiSize.y / 2.f;

    sf::RectangleShape dim({ uiSize.x, uiSize.y });
    dim.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(dim);

    // Текст "ПАУЗА"
    std::string ps = "ПАУЗА";
    sf::Text pauseTxt(font, sf::String::fromUtf8(ps.begin(), ps.end()), 128);
    pauseTxt.setFillColor(sf::Color::White);
    pauseTxt.setPosition({cx - pauseTxt.getLocalBounds().size.x / 2.f, cy - 180.f});
    window.draw(pauseTxt);

    // Пересчитываем позиции кнопок (зависят от размера окна)
    computePauseBtnLayout();

    sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window), uiView);

    auto drawPauseBtn = [&](const std::string& label, sf::FloatRect r) {
        bool hov = r.contains(mouse);
        sf::RectangleShape bg(r.size);
        bg.setPosition(r.position);
        bg.setFillColor(hov ? sf::Color(72, 72, 72, 230) : Colors::slotBg);
        window.draw(bg);

        sf::Text t(font, sf::String::fromUtf8(label.begin(), label.end()), 24);
        t.setFillColor(sf::Color::White);
        t.setPosition({
            r.position.x + (r.size.x - t.getLocalBounds().size.x) / 2.f,
            r.position.y + (r.size.y - t.getLocalBounds().size.y) / 2.f - 4.f
        });
        window.draw(t);
    };

    drawPauseBtn("Завершить", pauseMenuRect);
    drawPauseBtn("Заново",       pauseRestartRect);
    drawPauseBtn("Продолжить",   pauseContinueRect);
}

// Метод отрисовки экрана победы / поражения
void Game::renderEndScreen() {
    auto& font = ResourceManager::getFont("main");
    auto ws    = uiView.getSize();
    float cx   = ws.x / 2.f;
    float cy   = ws.y / 2.f;
    sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window), uiView);

    // Затемнение
    sf::RectangleShape dimRect;
    dimRect.setSize(sf::Vector2f(ws));
    dimRect.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(dimRect);

    // Заголовок
    bool isWin = (state == GameState::Victory);
    std::string header = isWin ? "ПОБЕДА!" : "ПОРАЖЕНИЕ";
    sf::Color   hColor = isWin ? Colors::moneyText : Colors::livesText;

    sf::Text hTxt(font, sf::String::fromUtf8(header.begin(), header.end()), 112);
    hTxt.setFillColor(hColor);
    hTxt.setPosition({cx - hTxt.getLocalBounds().size.x / 2.f, cy - 200.f});
    window.draw(hTxt);

    // Подпись
    std::string sub = isWin ? "Все волны отражены!" : "База уничтожена";
    sf::Text subTxt(font, sf::String::fromUtf8(sub.begin(), sub.end()), 30);
    subTxt.setFillColor(sf::Color(190, 190, 190, 220));
    subTxt.setPosition({cx - subTxt.getLocalBounds().size.x / 2.f, cy - 70.f});
    window.draw(subTxt);

    // Кнопки
    float btnW = 230.f, btnH = 60.f, btnGap = 24.f;
    float totalW = btnW * 2 + btnGap;
    endMenuRect    = sf::FloatRect({cx - totalW / 2.f,           cy + 40.f}, {btnW, btnH});
    endRestartRect = sf::FloatRect({cx + btnGap / 2.f,           cy + 40.f}, {btnW, btnH});

    auto drawEndBtn = [&](const std::string& label, sf::FloatRect r) {
        bool hov = r.contains(mouse);
        sf::RectangleShape bg(r.size);
        bg.setPosition(r.position);
        bg.setFillColor(hov ? sf::Color(72, 72, 72, 230) : Colors::slotBg);
        window.draw(bg);
        sf::Text t(font, sf::String::fromUtf8(label.begin(), label.end()), 24);
        t.setFillColor(sf::Color::White);
        t.setPosition({
            r.position.x + (r.size.x - t.getLocalBounds().size.x) / 2.f,
            r.position.y + (r.size.y - t.getLocalBounds().size.y) / 2.f - 4.f
        });
        window.draw(t);
    };

    drawEndBtn("Вернуться", endMenuRect);
    drawEndBtn("Заново",       endRestartRect);
}


void Game::clampView() {
    sf::Vector2f center = worldView.getCenter();
    sf::Vector2f size = worldView.getSize();

    // Физические границы карты в игровом мире
    float mapLeft = map.getMapOffset().x;
    float mapTop = map.getMapOffset().y;
    float mapRight = mapLeft + map.getWidth() * 64.f;
    float mapBottom = mapTop + map.getHeight() * 64.f;

    float marginX = size.x * 0.1f;
    float marginY = size.y * 0.1f;

    // Ограничиваем X
    if (center.x < mapLeft - marginX) center.x = mapLeft - marginX;
    if (center.x > mapRight + marginX) center.x = mapRight + marginX;

    // Ограничиваем Y
    if (center.y < mapTop - marginY) center.y = mapTop - marginY;
    if (center.y > mapBottom + marginY) center.y = mapBottom + marginY;

    worldView.setCenter(center);
}