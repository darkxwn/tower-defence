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
    : window(window), base(sf::Vector2i{0, 0})
{
    map.load(levelPath);
    map.centerOnScreen(window.getSize(), 75.f, 120.f);
    money = map.getStartMoney();
    base  = Base(map.getBasePos());
    waveSystem.loadWaves(levelPath);
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
    auto ws     = window.getSize();
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

// ─────────────────────────────────────────────────────────────────────────────
//  Обработка событий
// ─────────────────────────────────────────────────────────────────────────────
void Game::handleEvents() {
    while (std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            window.close();

        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Space)
                waveSystem.startWave();

            if (key->code == sf::Keyboard::Key::Escape) {
                if (state == GameState::Playing)       state = GameState::Paused;
                else if (state == GameState::Paused)   state = GameState::Playing;
            }

            if (key->code == sf::Keyboard::Key::P) {
                if (state == GameState::Playing)       state = GameState::Paused;
                else if (state == GameState::Paused)   state = GameState::Playing;
            }
        }

        if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            sf::Vector2u newSize = {resized->size.x, resized->size.y};
            map.centerOnScreen(newSize, 75.f, 120.f);
            window.setView(sf::View(sf::FloatRect({0.f, 0.f}, sf::Vector2f(newSize))));
        }

        if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (click->button != sf::Mouse::Button::Left) continue;
            sf::Vector2f mousePos = sf::Vector2f(click->position);

            // ── Экран результата (Victory / GameOver) ──
            if (state == GameState::Victory || state == GameState::GameOver) {
                if (endMenuRect.contains(mousePos)) {
                    endReason = GameEndReason::ReturnToMenu;
                } else if (endRestartRect.contains(mousePos)) {
                    endReason = GameEndReason::Restart;
                }
                continue;
            }

            // ── Оверлей паузы ──
            if (state == GameState::Paused) {
                if (pauseContinueRect.contains(mousePos)) {
                    state = GameState::Playing;
                } else if (pauseRestartRect.contains(mousePos)) {
                    endReason = GameEndReason::Restart;      // <-- только Restart, не меню
                } else if (pauseMenuRect.contains(mousePos)) {
                    endReason = GameEndReason::ReturnToMenu;
                }
                continue;
            }

            // ── Игровой HUD ──
            hud.handleClick(mousePos);

            if (hud.isPauseClicked()) {
                state = (state == GameState::Playing) ? GameState::Paused : GameState::Playing;
            }
            if (hud.isSkipClicked())
                waveSystem.startWave();

            // ── Расстановка башен ──
            if (state == GameState::Playing) {
                Tile* tile = map.getTileAtScreen(mousePos);
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
                                if      (name == "cannon") type = TowerType::Cannon;
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

// ─────────────────────────────────────────────────────────────────────────────
//  Обновление
// ─────────────────────────────────────────────────────────────────────────────
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

// ─────────────────────────────────────────────────────────────────────────────
//  Отрисовка
// ─────────────────────────────────────────────────────────────────────────────
void Game::render() {
    window.clear(Colors::gameBg);

    map.render(window);

    hud.render(window, money, base.getLives(),
               waveSystem.getCurrentWave(), waveSystem.getState());

    Tile* selected = map.getSelectedTile();
    for (auto& t : towers) {
        bool showR = selected && selected->gridPos == t.getGridPos();
        t.render(window, map.getMapOffset(), showR);
    }
    for (auto& e : enemies)
        e.render(window, map.getMapOffset());

    if (state == GameState::Paused)              renderPauseOverlay();
    if (state == GameState::Victory ||
        state == GameState::GameOver)             renderEndScreen();

    window.display();
}

//  Оверлей паузы
void Game::renderPauseOverlay() {
    auto& font = ResourceManager::getFont("main");
    auto ws    = window.getSize();
    float cx   = ws.x / 2.f;
    float cy   = ws.y / 2.f;

    // Затемнение
    sf::RectangleShape dimRect;
    dimRect.setSize(sf::Vector2f(ws));
    dimRect.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(dimRect);

    // Текст "ПАУЗА"
    std::string ps = "ПАУЗА";
    sf::Text pauseTxt(font, sf::String::fromUtf8(ps.begin(), ps.end()), 128);
    pauseTxt.setFillColor(sf::Color::White);
    pauseTxt.setPosition({cx - pauseTxt.getLocalBounds().size.x / 2.f, cy - 180.f});
    window.draw(pauseTxt);

    // Пересчитываем позиции кнопок (зависят от размера окна)
    computePauseBtnLayout();

    sf::Vector2f mouse = sf::Vector2f(sf::Mouse::getPosition(window));

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

//  Экран победы / поражения
void Game::renderEndScreen() {
    auto& font = ResourceManager::getFont("main");
    auto ws    = window.getSize();
    float cx   = ws.x / 2.f;
    float cy   = ws.y / 2.f;
    sf::Vector2f mouse = sf::Vector2f(sf::Mouse::getPosition(window));

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
