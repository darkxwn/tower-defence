#include <SFML/Graphics.hpp>
#include "Menu.hpp"
#include "Game.hpp"
#include "GameData.hpp"
#include "ResourceManager.hpp"

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <unistd.h>
#endif


// Загружает все общие ресурсы (шрифты, текстуры, статы)
// Вызывается один раз при старте приложения
static void loadResources() {
#ifdef __APPLE__
    // Этот код находит папку Resources внутри твоего .app
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[1024];
    if (CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8*)path, 1024)) {
        chdir(path); // Переходим в папку Resources
    }
    CFRelease(resourcesURL);
#endif
    // Шрифт
    ResourceManager::loadFont("main", "assets/fonts/web_ibm_mda.ttf");

    // Иконки интерфейса
    ResourceManager::load("icon-pause",    "assets/icons/pause.png");
    ResourceManager::load("icon-skip",     "assets/icons/play.png");
    ResourceManager::load("icon-coins",    "assets/icons/coins.png");
    ResourceManager::load("icon-heart",    "assets/icons/heart.png");
    ResourceManager::load("icon-play",     "assets/icons/play.png");
    ResourceManager::load("icon-upgrades", "assets/icons/upgrades.png");
    ResourceManager::load("icon-settings", "assets/icons/settings.png");
    ResourceManager::load("icon-exit",     "assets/icons/exit.png");

    // Спрайты тайлов карты
    ResourceManager::load("road",     "assets/sprites/tile-road.png");
    ResourceManager::load("platform", "assets/sprites/tile-platform.png");
    ResourceManager::load("portal",   "assets/sprites/tile-portal.png");
    ResourceManager::load("base",     "assets/sprites/tile-base.png");
    ResourceManager::load("active",   "assets/sprites/tile-active-layer.png");

    // Спрайты врагов
    ResourceManager::load("enemy-basic",   "assets/sprites/enemy-basic.png");
    ResourceManager::load("enemy-fast",    "assets/sprites/enemy-fast.png");
    ResourceManager::load("enemy-strong",  "assets/sprites/enemy-strong.png");

    // Загружаем статы башен и врагов из конфигов
    GameData::load();

    // Спрайты башен — автоматически по именам из GameData
    for (const auto& name : GameData::getTowerNames()) {
        ResourceManager::load("tower-" + name + "-base",    "assets/sprites/tower-" + name + "-base.png");
        ResourceManager::load("tower-" + name + "-turret",  "assets/sprites/tower-" + name + "-turret.png");
        ResourceManager::load("tower-" + name + "-preview", "assets/sprites/tower-" + name + "-preview.png");
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

    // Главный цикл приложения
    Menu menu(window);

    while (window.isOpen()) {
        // Показываем меню пока не выбран уровень
        while (window.isOpen() && !menu.isLevelChosen()) {
            menu.handleEvents();
            menu.render();
        }

        if (!window.isOpen()) break;

        // Запускаем игровую сессию с выбранным уровнем
        std::string levelPath = menu.getChosenLevel();
        menu.resetChoice();

        Game game(window, levelPath);
        game.run();
    }

    return 0;
}
