#include <SFML/Graphics.hpp>
#include "Menu.hpp"
#include "utils/Logger.hpp"
#include "Game.hpp"
#include "GameData.hpp"
#include "ResourceManager.hpp"
#include "SettingsManager.hpp"
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////
//
// ТОЧКА ВХОДА
//
///////////////////////////////////////////////////////////////////////////

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
    ResourceManager::load("icon-pause", assetsPath + "icons/pause.png");
    ResourceManager::load("icon-skip", assetsPath + "icons/play.png");
    ResourceManager::load("icon-coins", assetsPath + "icons/coins.png");
    ResourceManager::load("icon-heart", assetsPath + "icons/heart.png");
    ResourceManager::load("icon-play", assetsPath + "icons/play.png");
    ResourceManager::load("icon-upgrades", assetsPath + "icons/upgrades.png");
    ResourceManager::load("icon-settings", assetsPath + "icons/settings.png");
    ResourceManager::load("icon-exit", assetsPath + "icons/exit.png");
    ResourceManager::load("icon-speed1", assetsPath + "icons/icon-speed1.png");
    ResourceManager::load("icon-speed2", assetsPath + "icons/icon-speed2.png");
    ResourceManager::load("icon-speed3", assetsPath + "icons/icon-speed3.png");

    // ТАЙЛЫ
    ResourceManager::load("road", assetsPath + "sprites/tile-road.png");
    ResourceManager::load("platform", assetsPath + "sprites/tile-platform.png");
    ResourceManager::load("active", assetsPath + "sprites/tile-active-layer.png");
    ResourceManager::load("portal", assetsPath + "sprites/tile-portal.png");
    ResourceManager::load("portal-layer1", assetsPath + "sprites/tile-portal-layer1.png");
    ResourceManager::load("portal-layer2", assetsPath + "sprites/tile-portal-layer2.png");
    ResourceManager::load("base", assetsPath + "sprites/tile-base.png");

    // ВРАГИ
    std::vector<std::string> enemyTypes = { "basic", "fast", "strong" };
    for (const auto& type : enemyTypes) {
        ResourceManager::load("enemy-" + type, assetsPath + "sprites/enemy-" + type + ".png");
    }

    // БАШНИ
    auto towerNames = GameData::getTowerNames();
    for (const auto& name : towerNames) {
        std::vector<std::string> parts = { "base", "turret", "proj", "preview" };
        for (const auto& part : parts) {
            std::string resId = "tower-" + name + "-" + part;
            ResourceManager::load(resId, assetsPath + "sprites/" + resId + ".png");
        }
    }
}

int main() {
    SettingsManager settings;
    settings.load();
    
    sf::RenderWindow window(sf::VideoMode({1280, 720}), "Tower Defence");
    window.setFramerateLimit(60);
    window.setMinimumSize(sf::Vector2u(1280, 720));

    loadResources();

    auto menu = std::make_unique<Menu>(window, settings);

    while (window.isOpen()) {
        while (window.isOpen() && !menu->isLevelChosen()) {
            menu->handleEvents();
            if (!window.isOpen()) { menu.reset(); return 0; }

            if (menu->consumesWindowRecreationRequest()) {
                bool fs = settings.get<bool>("fullscreen");
                window.create(sf::VideoMode({1280, 720}), "Tower Defence", fs ? sf::State::Fullscreen : sf::State::Windowed);
                window.setFramerateLimit(60);
                menu->updateViewSizes(window.getSize());
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
            Game game(window, settings, levelPath);
            game.run();

            GameEndReason reason = game.getEndReason();
            game.cleanup(); // Очищаем контейнеры перед уничтожением объекта

            if (reason == GameEndReason::ReturnToMenu) {
                keepPlaying = false;
            } else if (reason == GameEndReason::Restart) {
                continue; 
            } else if (reason == GameEndReason::Win || reason == GameEndReason::Lose) {
                menu->notifyResult(reason == GameEndReason::Win ? SessionResult::Win : SessionResult::Lose, levelPath);
                keepPlaying = false;
            } else {
                keepPlaying = false;
            }
        }
        
        if (window.isOpen()) {
            menu->updateViewSizes(window.getSize());
        }
    }

    menu.reset();
    return 0;
}
