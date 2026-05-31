#include <SFML/Graphics.hpp>
#include "Menu.hpp"
#include "utils/Logger.hpp"
#include "Game.hpp"
#include "GameData.hpp"
#include "ResourceManager.hpp"
#include "SettingsManager.hpp"
#include "SaveManager.hpp"
#include "utils/PathResolver.hpp"
#include <string>
#include <vector>
#include <filesystem>

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

static void loadResources() {
    // Получаем платформонезависимый путь к папке assets
    std::filesystem::path assetsPath = PathResolver::getResourcesPath("assets");

    // Вспомогательная лямбда, чтобы не переписывать строки загрузки вручную
    auto resolve = [&](const std::string& relativePath) -> std::string {
#ifdef __ANDROID__
        // На Android SFML 3 читает ресурсы через AssetManager по относительному пути "assets/..."
        return "assets/" + relativePath;
#else
        return (assetsPath / relativePath).string();
#endif
    };

    // ВНИМАНИЕ: Если GameData::load() внутри себя читает файлы (например, из папки "data"),
    // ему тоже нужно передать правильный путь через PathResolver::getResourcesPath("data")!
    GameData::load();

    // ШРИФТ
    ResourceManager::loadFont("main", resolve("fonts/web_ibm_mda.ttf"));

    // ИКОНКИ
    ResourceManager::load("icon-coins", resolve("icons/coins.png"));
    ResourceManager::load("icon-heart", resolve("icons/heart.png"));
    ResourceManager::load("icon-speed1", resolve("icons/icon-speed1.png"));
    ResourceManager::load("icon-speed2", resolve("icons/icon-speed2.png"));
    ResourceManager::load("icon-speed3", resolve("icons/icon-speed3.png"));
    ResourceManager::load("icon-start", resolve("icons/start.png"));
    ResourceManager::load("icon-pause", resolve("icons/pause.png"));
    ResourceManager::load("icon-play", resolve("icons/play.png"));
    ResourceManager::load("icon-upgrades", resolve("icons/upgrades.png"));
    ResourceManager::load("icon-settings", resolve("icons/settings.png"));
    ResourceManager::load("icon-exit", resolve("icons/exit.png"));
    ResourceManager::load("icon-save", resolve("icons/save.png"));
    ResourceManager::load("icon-back", resolve("icons/back.png"));
    ResourceManager::load("icon-audio", resolve("icons/audio.png"));
    ResourceManager::load("icon-music", resolve("icons/music.png"));
    ResourceManager::load("icon-level", resolve("icons/level.png"));
    ResourceManager::load("icon-chart", resolve("icons/chart.png"));
    ResourceManager::load("icon-sell", resolve("icons/sell.png"));
    ResourceManager::load("icon-upgrade", resolve("icons/upgrade.png"));
    ResourceManager::load("icon-upgrade2", resolve("icons/upgrade2.png"));
    ResourceManager::load("icon-money", resolve("icons/money.png"));
    ResourceManager::load("icon-star-empty", resolve("icons/star-empty.png"));
    ResourceManager::load("icon-star-filled", resolve("icons/star-filled.png"));
    ResourceManager::load("icon-hit", resolve("icons/hit.png"));
    ResourceManager::load("icon-brush", resolve("icons/brush.png"));
    ResourceManager::load("icon-brush-active", resolve("icons/brush-active.png"));
    ResourceManager::load("icon-explosion", resolve("icons/explosion.png"));
    ResourceManager::load("icon-animation", resolve("icons/animation.png"));
    ResourceManager::load("icon-trail", resolve("icons/trail.png"));

    // СПРАЙТЫ ИНТЕРФЕЙСА
    ResourceManager::load("button", resolve("sprites/button.png"), false);
    ResourceManager::load("button-hover", resolve("sprites/button-hover.png"), false);
    ResourceManager::load("button-active", resolve("sprites/button-active.png"), false);
    ResourceManager::load("button-disabled", resolve("sprites/button-disabled.png"), false);
    ResourceManager::load("button-flat", resolve("sprites/button-flat.png"), false);
    ResourceManager::load("button-flat-hover", resolve("sprites/button-flat-hover.png"), false);
    ResourceManager::load("button-flat-disabled", resolve("sprites/button-flat-disabled.png"), false);
    ResourceManager::load("panel", resolve("sprites/panel.png"), false);
    ResourceManager::load("panel-light", resolve("sprites/panel-light.png"), false);
    ResourceManager::load("panel-lighter", resolve("sprites/panel-lighter.png"));
    ResourceManager::load("main-layer", resolve("sprites/main-layer.png"), false);
    ResourceManager::load("card", resolve("sprites/card.png"), false);
    ResourceManager::load("card-light", resolve("sprites/card-light.png"), false);
    ResourceManager::load("card-dark", resolve("sprites/card-dark.png"), false);
    ResourceManager::load("card-select", resolve("sprites/card-select.png"), false);

#ifdef __ANDROID__
    ResourceManager::load("icon-sensivity", resolve("icons/sensivity-mobile.png"));
    ResourceManager::load("icon-display", resolve("icons/display-mobile.png"));
#else
    ResourceManager::load("icon-sensivity", resolve("icons/sensivity-desktop.png"));
    ResourceManager::load("icon-display", resolve("icons/display-desktop.png"));
    ResourceManager::load("icon-vsync", resolve("icons/vsync.png"));
    ResourceManager::load("icon-fullscreen", resolve("icons/fullscreen.png"));
#endif
    ResourceManager::load("icon-effect", resolve("icons/effect.png"));

    // ТАЙЛЫ
    ResourceManager::load("road", resolve("sprites/tile-road.png"));
    ResourceManager::load("platform", resolve("sprites/tile-platform.png"));
    ResourceManager::load("active", resolve("sprites/tile-active-layer.png"));
    ResourceManager::load("portal", resolve("sprites/tile-portal.png"));
    ResourceManager::load("portal-layer1", resolve("sprites/tile-portal-layer1.png"));
    ResourceManager::load("portal-layer2", resolve("sprites/tile-portal-layer2.png"));
    ResourceManager::load("base", resolve("sprites/tile-base.png"));

    // ВРАГИ 
    for (const auto& type : GameData::getEnemyTypes()) {
        ResourceManager::load("enemy-" + type, resolve("sprites/enemy-" + type + ".png"));
    }

    // БАШНИ
    auto towerNames = GameData::getTowerNames();
    std::vector<std::string> parts = { "base", "turret", "proj", "preview" };
    for (const auto& name : towerNames) {
        for (const auto& part : parts) {
            std::string resId = "tower-" + name + "-" + part;
            ResourceManager::load(resId, resolve("sprites/" + resId + ".png"));
        }
    }
}

int main() {
    // Пишем логи в безопасную пользовательскую директорию
    std::string logPath = PathResolver::getWriteablePath("logs/latest.log").string();
    Engine::Logger::init(logPath);

    SettingsManager settings;
    SaveManager saveManager;
    Engine::VFXManager vfxManager;

    // ВАЖНО: Если твои менеджеры сохранений/настроек поддерживают передачу пути,
    // передай им пути из PathResolver, например:
    // settings.load(PathResolver::getWriteablePath("settings.json").string());
    // saveManager.load(PathResolver::getWriteablePath("save.json").string());
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
        
#ifndef __ANDROID__
        levelPath = (PathResolver::getResourcesPath("data") / "levels" / std::filesystem::path(levelPath).filename()).string();
#endif

        menu->resetChoice();
        menu->resetLastResult();
        
        bool keepPlaying = true;
        while (keepPlaying && window.isOpen()) {
            Game game(window, settings, saveManager, menu->getUpgradeManager(), vfxManager, levelPath);
            game.run();

            GameEndReason reason = game.getEndReason();
            game.cleanup();

            if (reason == GameEndReason::ReturnToMenu) {
                keepPlaying = false;
            } else if (reason == GameEndReason::Restart) {
                continue; 
            } else {
                keepPlaying = false;
            }
        }
        
        if (window.isOpen()) {
            if (!isFullscreen) windowedSize = window.getSize();
            menu->refreshLevelCards();
            menu->updateViewSizes(window.getSize());
        }
    }

    menu.reset();
    return 0;
}