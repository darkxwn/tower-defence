#include <SFML/Graphics.hpp>
#include "Menu.hpp"
#include "Game.hpp"
#include "GameData.hpp"
#include "ResourceManager.hpp"

// Загрузка всех общих ресурсов (шрифты, текстуры, данные)
// Вызывается один раз при старте приложения
static void loadResources() {
    ResourceManager::loadFont("main", "assets/fonts/web_ibm_mda.ttf");

    ResourceManager::load("icon-pause", "assets/icons/pause.png");
    ResourceManager::load("icon-skip", "assets/icons/play.png");
    ResourceManager::load("icon-coins", "assets/icons/coins.png");
    ResourceManager::load("icon-heart", "assets/icons/heart.png");
    ResourceManager::load("icon-play", "assets/icons/play.png");
    ResourceManager::load("icon-upgrades", "assets/icons/upgrades.png");
    ResourceManager::load("icon-settings", "assets/icons/settings.png");
    ResourceManager::load("icon-exit", "assets/icons/exit.png");

    ResourceManager::load("road", "assets/sprites/tile-road.png");
    ResourceManager::load("platform", "assets/sprites/tile-platform.png");
    ResourceManager::load("portal", "assets/sprites/tile-portal.png");
    ResourceManager::load("base", "assets/sprites/tile-base.png");
    ResourceManager::load("active", "assets/sprites/tile-active-layer.png");

    ResourceManager::load("enemy-basic", "assets/sprites/enemy-basic.png");
    ResourceManager::load("enemy-fast", "assets/sprites/enemy-fast.png");
    ResourceManager::load("enemy-strong", "assets/sprites/enemy-strong.png");

    GameData::load();

    for (const auto& name : GameData::getTowerNames()) {
        ResourceManager::load("tower-" + name + "-base",
            "assets/sprites/tower-" + name + "-base.png");
        ResourceManager::load("tower-" + name + "-turret",
            "assets/sprites/tower-" + name + "-turret.png");
        ResourceManager::load("tower-" + name + "-preview",
            "assets/sprites/tower-" + name + "-preview.png");
    }
}

int main() {
    sf::RenderWindow window(
        sf::VideoMode({ 1920, 1080 }),
        "Tower Defence",
        sf::Style::Default
    );
    window.setFramerateLimit(60);
    window.setMinimumSize(sf::Vector2u({ 1280, 720 }));

    loadResources();

    Menu menu(window);

    // Главный цикл приложения: Меню ↔ Игра
    while (window.isOpen()) {

        // ── Показываем меню до тех пор, пока игрок не выберет уровень ──
        while (window.isOpen() && !menu.isLevelChosen()) {
            menu.handleEvents();
            menu.render();
        }
        if (!window.isOpen()) break;

        // ── Запускаем игровую сессию ──
        std::string levelPath = menu.getChosenLevel();
        menu.resetChoice(); // сбрасываем флаг, но остаёмся в LevelSelect

        bool keepPlaying = true;
        while (keepPlaying && window.isOpen()) {
            Game game(window, levelPath);
            game.run();

            GameEndReason reason = game.getEndReason();

            switch (reason) {
            case GameEndReason::Restart:
                // Пересоздаём Game с тем же уровнем — цикл повторится
                break;

            case GameEndReason::Win:
                menu.notifyResult(SessionResult::Win, levelPath);
                keepPlaying = false;
                break;

            case GameEndReason::Lose:
                menu.notifyResult(SessionResult::Lose, levelPath);
                keepPlaying = false;
                break;

            case GameEndReason::ReturnToMenu:
            default:
                // Возврат в меню без оверлея результата
                keepPlaying = false;
                break;
            }
        }
        // После выхода из keepPlaying — возвращаемся в меню
        // (menu уже настроен notifyResult или просто ждёт в LevelSelect)
    }

    return 0;
}