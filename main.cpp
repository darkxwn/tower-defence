#include <SFML/Graphics.hpp>
#include "Menu.hpp"
#include "Game.hpp"
#include "GameData.hpp"
#include "ResourceManager.hpp"
#include <string>

#ifdef __ANDROID__
#include <android/log.h>
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "tower-defence", __VA_ARGS__)
#else
#define LOGE(...) std::cerr << __VA_ARGS__ << std::endl
#endif

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <unistd.h>
#endif

static void loadResources() {
    // Используем "assets/" для ПК и "" для мобилок
    std::string assetsPath = "assets/";
    std::string dataPath = "data/";

#ifdef __APPLE__
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[1024];
    if (CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8*)path, 1024)) {
        chdir(path);
    }
    CFRelease(resourcesURL);
    assetsPath = "";
    dataPath = "";
#endif

#ifdef __ANDROID__
    assetsPath = "";
    dataPath = "";
#endif

    // ШРИФТ (Убедись, что нет лишних /)
    ResourceManager::loadFont("main", assetsPath + "fonts/web_ibm_mda.ttf");

    // ИКОНКИ
    ResourceManager::load("icon-pause", assetsPath + "icons/pause.png");
    ResourceManager::load("icon-skip", assetsPath + "icons/play.png");
    ResourceManager::load("icon-coins", assetsPath + "icons/coins.png");
    ResourceManager::load("icon-heart", assetsPath + "icons/heart.png");
    ResourceManager::load("icon-play", assetsPath + "icons/play.png");
    ResourceManager::load("icon-upgrades", assetsPath + "icons/upgrades.png");
    ResourceManager::load("icon-settings", assetsPath + "icons/settings.png");
    ResourceManager::load("icon-exit", assetsPath + "icons/exit.png");

    // ТАЙЛЫ
    ResourceManager::load("road", assetsPath + "sprites/tile-road.png");
    ResourceManager::load("platform", assetsPath + "sprites/tile-platform.png");
    ResourceManager::load("portal", assetsPath + "sprites/tile-portal.png");
    ResourceManager::load("base", assetsPath + "sprites/tile-base.png");
    ResourceManager::load("active", assetsPath + "sprites/tile-active-layer.png");

    // ВРАГИ (Убрал лишний / перед sprites)
    ResourceManager::load("enemy-basic", assetsPath + "sprites/enemy-basic.png");
    ResourceManager::load("enemy-fast", assetsPath + "sprites/enemy-fast.png");
    ResourceManager::load("enemy-strong", assetsPath + "sprites/enemy-strong.png");

    // ВАЖНО: На Android GameData::load() упадет, если использует std::ifstream!
    // Пока просто вызываем, но будь готов переделывать GameData
    GameData::load();

    for (const auto& name : GameData::getTowerNames()) {
        ResourceManager::load("tower-" + name + "-base", assetsPath + "sprites/tower-" + name + "-base.png");
        ResourceManager::load("tower-" + name + "-turret", assetsPath + "sprites/tower-" + name + "-turret.png");
        ResourceManager::load("tower-" + name + "-preview", assetsPath + "sprites/tower-" + name + "-preview.png");
    }
}

int main(int argc, char* argv[]) {
    // На Android окно ДОЛЖНО быть создано первым!
    sf::RenderWindow window(sf::VideoMode({ 1920, 1080 }), "Tower Defence");
    window.setFramerateLimit(60);

    // Только после создания окна можно грузить ресурсы на Android
    loadResources();

    Menu menu(window);

    while (window.isOpen()) {
        while (window.isOpen() && !menu.isLevelChosen()) {
            menu.handleEvents();
            menu.render();
        }

        if (!window.isOpen()) break;

        std::string levelPath = menu.getChosenLevel();
        menu.resetChoice();

        // На Android levelPath тоже должен быть без "data/" если ты грузишь через ifstream
        Game game(window, levelPath);
        game.run();
    }

    return 0;
}