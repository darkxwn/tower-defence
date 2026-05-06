#include <SFML/Graphics.hpp>
#include "Menu.hpp"
#include "utils/Logger.hpp"
#include "Game.hpp"
#include "GameData.hpp"
#include "ResourceManager.hpp"
#include "SettingsManager.hpp"
#include "SaveManager.hpp"
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////
//
// ТОЧКА ВХОДА
//
///////////////////////////////////////////////////////////////////////////

using namespace Engine;

#ifdef __ANDROID__
#include <android/native_activity.h>
#include <SFML/System/NativeActivity.hpp>

static void setImmersiveMode(sf::RenderWindow& window) {
    ANativeActivity* activity = sf::getNativeActivity();
    JavaVM* vm = activity->vm;
    JNIEnv* env = nullptr;
    vm->AttachCurrentThread(&env, nullptr);
    jobject activityObj = activity->clazz;
    jclass activityClass = env->GetObjectClass(activityObj);
    jmethodID getWindow = env->GetMethodID(activityClass, "getWindow", "()Landroid/view/Window;");
    jobject windowObj = env->CallObjectMethod(activityObj, getWindow);
    jclass windowClass = env->GetObjectClass(windowObj);
    jmethodID getDecorView = env->GetMethodID(windowClass, "getDecorView", "()Landroid/view/View;");
    jobject decorViewObj = env->CallObjectMethod(windowObj, getDecorView);
    jclass viewClass = env->GetObjectClass(decorViewObj);
    jmethodID setSystemUiVisibility = env->GetMethodID(viewClass, "setSystemUiVisibility", "(I)V");
    env->CallVoidMethod(decorViewObj, setSystemUiVisibility, 5894);
    vm->DetachCurrentThread();
}
#endif

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <unistd.h>
#endif

static void loadResources() {
    std::string assetsPath = "assets/";
#ifdef __APPLE__
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[1024];
    if (CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8*)path, 1024)) chdir(path);
    CFRelease(resourcesURL);
#endif
#ifdef __ANDROID__
    assetsPath = "";
#endif

    GameData::load();

    // ШРИФТ
    ResourceManager::loadFont("main", assetsPath + "fonts/web_ibm_mda.ttf");

    // ИКОНКИ
    ResourceManager::load("icon-coins", assetsPath + "icons/coins.png");
    ResourceManager::load("icon-heart", assetsPath + "icons/heart.png");
    ResourceManager::load("icon-speed1", assetsPath + "icons/icon-speed1.png");
    ResourceManager::load("icon-speed2", assetsPath + "icons/icon-speed2.png");
    ResourceManager::load("icon-speed3", assetsPath + "icons/icon-speed3.png");
    ResourceManager::load("icon-start", assetsPath + "icons/start.png");
    ResourceManager::load("icon-pause", assetsPath + "icons/pause.png");
    ResourceManager::load("icon-play", assetsPath + "icons/play.png");
    ResourceManager::load("icon-upgrades", assetsPath + "icons/upgrades.png");
    ResourceManager::load("icon-settings", assetsPath + "icons/settings.png");
    ResourceManager::load("icon-exit", assetsPath + "icons/exit.png");
    ResourceManager::load("icon-save", assetsPath + "icons/save.png");
    ResourceManager::load("icon-back", assetsPath + "icons/back.png");
    ResourceManager::load("icon-audio", assetsPath + "icons/audio.png");
    ResourceManager::load("icon-music", assetsPath + "icons/music.png");
    ResourceManager::load("icon-level", assetsPath + "icons/level.png");
    ResourceManager::load("icon-chart", assetsPath + "icons/chart.png");
    ResourceManager::load("icon-sell", assetsPath + "icons/sell.png");
    ResourceManager::load("icon-upgrade", assetsPath + "icons/upgrade.png");
    ResourceManager::load("icon-upgrade2", assetsPath + "icons/upgrade2.png");
    ResourceManager::load("icon-money", assetsPath + "icons/money.png");
    ResourceManager::load("icon-star-empty", assetsPath + "icons/star-empty.png");
    ResourceManager::load("icon-star-filled", assetsPath + "icons/star-filled.png");
    ResourceManager::load("icon-hit", assetsPath + "icons/hit.png");
    ResourceManager::load("icon-explosion", assetsPath + "icons/explosion.png");
    ResourceManager::load("icon-animation", assetsPath + "icons/animation.png");
    ResourceManager::load("icon-trail", assetsPath + "icons/trail.png");


    // СПРАЙТЫ ИНТЕРФЕЙСА
    ResourceManager::load("button", assetsPath + "sprites/button.png", false);
    ResourceManager::load("button-hover", assetsPath + "sprites/button-hover.png", false);
    ResourceManager::load("button-active", assetsPath + "sprites/button-active.png", false);
    ResourceManager::load("button-disabled", assetsPath + "sprites/button-disabled.png", false);
    ResourceManager::load("button-flat", assetsPath + "sprites/button-flat.png", false);
    ResourceManager::load("button-flat-hover", assetsPath + "sprites/button-flat-hover.png", false);
    ResourceManager::load("button-flat-disabled", assetsPath + "sprites/button-flat-disabled.png", false);
    ResourceManager::load("panel", assetsPath + "sprites/panel.png", false);
    ResourceManager::load("panel-light", assetsPath + "sprites/panel-light.png", false);
    ResourceManager::load("panel-lighter", assetsPath + "sprites/panel-lighter.png");
    ResourceManager::load("main-layer", assetsPath + "sprites/main-layer.png", false);
    ResourceManager::load("card", assetsPath + "sprites/card.png", false);
    ResourceManager::load("card-light", assetsPath + "sprites/card-light.png", false);
    ResourceManager::load("card-dark", assetsPath + "sprites/card-dark.png", false);
    ResourceManager::load("card-select", assetsPath + "sprites/card-select.png", false);

#ifdef __ANDROID__
    ResourceManager::load("icon-sensivity", assetsPath + "icons/sensivity-mobile.png");
    ResourceManager::load("icon-display", assetsPath + "icons/display-mobile.png");
#else
    ResourceManager::load("icon-sensivity", assetsPath + "icons/sensivity-desktop.png");
    ResourceManager::load("icon-display", assetsPath + "icons/display-desktop.png");
    ResourceManager::load("icon-vsync", assetsPath + "icons/vsync.png");
    ResourceManager::load("icon-fullscreen", assetsPath + "icons/fullscreen.png");
#endif
    ResourceManager::load("icon-effect", assetsPath + "icons/effect.png");

    // ТАЙЛЫ
    ResourceManager::load("road", assetsPath + "sprites/tile-road.png");
    ResourceManager::load("platform", assetsPath + "sprites/tile-platform.png");
    ResourceManager::load("active", assetsPath + "sprites/tile-active-layer.png");
    ResourceManager::load("portal", assetsPath + "sprites/tile-portal.png");
    ResourceManager::load("portal-layer1", assetsPath + "sprites/tile-portal-layer1.png");
    ResourceManager::load("portal-layer2", assetsPath + "sprites/tile-portal-layer2.png");
    ResourceManager::load("base", assetsPath + "sprites/tile-base.png");

    // ВРАГИ 
    for (const auto& type : GameData::getEnemyTypes()) {
        ResourceManager::load("enemy-" + type, assetsPath + "sprites/enemy-" + type + ".png");
    }

    // БАШНИ
    auto towerNames = GameData::getTowerNames();
    std::vector<std::string> parts = { "base", "turret", "proj", "preview" };
    for (const auto& name : towerNames) {
        for (const auto& part : parts) {
            std::string resId = "tower-" + name + "-" + part;
            ResourceManager::load(resId, assetsPath + "sprites/" + resId + ".png");
        }
    }
}

int main() {
    Engine::Logger::init("logs/latest.log");
    SettingsManager settings;
    SaveManager saveManager;
    Engine::VFXManager vfxManager;

    settings.load();
    saveManager.load();
    
    sf::RenderWindow window(sf::VideoMode({1280, 720}), "Project: Gyurza", settings.get<bool>("fullscreen", false) ? sf::State::Fullscreen : sf::State::Windowed);
    window.setFramerateLimit(60);
    window.setMinimumSize(sf::Vector2u(1280, 720));

#ifdef __ANDROID__
    setImmersiveMode(window);
#endif

    loadResources();

    auto menu = std::make_unique<Menu>(window, settings, saveManager);

    bool isFullscreen = settings.get<bool>("fullscreen", false);
    sf::Vector2u windowedSize = { 1280, 720 };
    if (!isFullscreen) windowedSize = window.getSize();

    while (window.isOpen()) {
        while (window.isOpen() && !menu->isLevelChosen()) {
            menu->handleEvents();
            if (!window.isOpen()) { menu.reset(); return 0; }

            if (!isFullscreen) {
                windowedSize = window.getSize();
            }

            if (menu->consumesWindowRecreationRequest()) {
                menu->cleanup();
                menu.reset();

                bool fs = settings.get<bool>("fullscreen", false);
                isFullscreen = fs;

                // При полноэкранном режиме используем текущее разрешение экрана
                sf::VideoMode videoMode; 
                if (fs) {
                    videoMode = sf::VideoMode::getDesktopMode();
                } else {
                    videoMode = sf::VideoMode(windowedSize);
                }
                window.create(videoMode, "Project: Gyurza", fs ? sf::State::Fullscreen : sf::State::Windowed);
                window.setVerticalSyncEnabled(settings.get<bool>("vsync", true));
                window.setMinimumSize(sf::Vector2u(1280, 720));

                sf::Vector2u actualSize = window.getSize();
                menu = std::make_unique<Menu>(window, settings, saveManager);
                menu->updateViewSizes(actualSize);
                Logger::info("Окно пересоздано: {}x{}, Fullscreen: {}", actualSize.x, actualSize.y, fs);
            }

            menu->render();
            if (!window.isOpen()) { menu.reset(); return 0; }
        }
        if (!window.isOpen()) { menu.reset(); return 0; }

        std::string levelPath = menu->getChosenLevel();
        menu->resetChoice();
        menu->resetLastResult();
        
        bool keepPlaying = true;
        while (keepPlaying && window.isOpen()) {
            Game game(window, settings, saveManager, menu->getUpgradeManager(), vfxManager, levelPath);
            game.run();

            GameEndReason reason = game.getEndReason();
            game.cleanup(); // Очищаем контейнеры перед уничтожением объекта

            if (reason == GameEndReason::ReturnToMenu) {
                keepPlaying = false;
            } else if (reason == GameEndReason::Restart) {
                continue; 
            } else {
                keepPlaying = false;
            }
        }
        
        // Обновляем карточки уровней после выхода из игры
        if (window.isOpen()) {
            if (!isFullscreen) windowedSize = window.getSize();
            menu->refreshLevelCards();
            menu->updateViewSizes(window.getSize());
        }
    }

    menu.reset();
    return 0;
}
