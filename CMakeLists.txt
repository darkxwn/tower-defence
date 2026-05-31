cmake_minimum_required(VERSION 3.15...3.30)

set(GAME_VERSION_FULL "0.7.4b")

project(tower-defence 
    VERSION 0.6.2
    LANGUAGES CXX
)

# Для использования системных библиотек в Linux'ах
if(UNIX AND NOT APPLE AND NOT ANDROID)
    set(USE_SYSTEM_DEPS_DEFAULT ON)
else()
    set(USE_SYSTEM_DEPS_DEFAULT OFF)
endif()

option(USE_SYSTEM_DEPS "Using system libs (SFML, JSON)" ${USE_SYSTEM_DEPS_DEFAULT})

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/includes/Version.hpp.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/includes/Version.hpp"
)

if(MSVC)
    add_compile_options(/utf-8)
endif()

if(ANDROID)
    set(CMAKE_CXX_STANDARD 20 CACHE STRING "" FORCE)
    set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "" FORCE)
    
    # Убираем статик-линк, оставляем только shared для NDK
    set(CMAKE_ANDROID_STL_TYPE c++_shared CACHE STRING "" FORCE)
    set(ANDROID_STL c++_shared CACHE STRING "" FORCE)
    
    set(SFML_OS_ANDROID ON CACHE BOOL "" FORCE)
    set(SFML_USE_SYSTEM_DEPS OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_MAIN ON CACHE BOOL "" FORCE)
    
    # Глобальная линковка STL
    link_libraries(c++_shared)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
if(ANDROID)
    set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
endif()

if(USE_SYSTEM_DEPS)
    message(STATUS "Using system libs...")
    find_package(SFML 3.0 COMPONENTS Graphics Window System REQUIRED)
    find_package(nlohmann_json 3.12 REQUIRED)
else()
    message(STATUS "Depends downloads through FetchContent...")
    include(FetchContent)

    # JSON
    FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/nlohmann/json
        GIT_SHALLOW ON
        GIT_TAG v3.12.0
    )

    # SFML 3
    FetchContent_Declare(sfml
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.2
        GIT_SHALLOW ON
        SYSTEM
    )

    set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)
    set(SFML_INSTALL_PKGCONFIG_FILES OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)

    # ИСПРАВЛЕНО: соответствие имен
    FetchContent_MakeAvailable(sfml json)
endif()

# СПИСОК ИСХОДНЫХ ФАЙЛОВ
set(SOURCES
    main.cpp
    src/Base.cpp
    src/Enemy.cpp
    src/Tower.cpp
    src/Projectile.cpp
    src/Game.cpp
    src/HUD.cpp
    src/Map.cpp
    src/Menu.cpp
    src/GameData.cpp
    src/ResourceManager.cpp
    src/SettingsManager.cpp
    src/SaveManager.cpp
    src/WaveSystem.cpp
    src/UpgradeManager.cpp
    src/VFXManager.cpp
    src/utils/FileReader.cpp
    src/utils/Logger.cpp
    src/utils/PathResolver.cpp
    src/ui/Text.cpp
    src/ui/Button.cpp
    src/ui/NineSlice.cpp
    src/ui/Widget.cpp
    src/ui/Container.cpp
    src/ui/Slider.cpp
    src/ui/Toggle.cpp
    src/ui/Image.cpp
)

# Ресурсы для Windows и macOS
set(WIN_RESOURCES "")
if(WIN32)
    enable_language(RC)
    # Если у тебя есть файл .rc для иконки Windows, добавь его сюда
    set(WIN_RESOURCES "src/resource.rc")
endif()

set(MACOS_ICON "${CMAKE_CURRENT_SOURCE_DIR}/macos/game_macos.icns")

# СОЗДАНИЕ ЦЕЛИ
if(ANDROID)
    add_library(${PROJECT_NAME} SHARED ${SOURCES})
else()
    if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES} ${WIN_RESOURCES} ${MACOS_ICON})
    else()
        add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${SOURCES} ${WIN_RESOURCES} ${MACOS_ICON})
    endif()
endif()

# НАСТРОЙКИ СПЕЦИФИЧНЫХ ПЛАТФОРМ
if(APPLE)
    # Упаковка иконки в .app
    set_source_files_properties(${MACOS_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "Project: Gyurza"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.darkxwn.billvlad.project-gyurza"
        MACOSX_BUNDLE_BUNDLE_VERSION "0.0.1"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
        MACOSX_BUNDLE_ICON_FILE "game_macos.icns"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/macos/Info.plist.in"
    )
endif()

if(WIN32)
    # Точка входа для графического приложения (отключает консоль)
    set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION} LINK_FLAGS "/ENTRY:mainCRTStartup")
endif()

# ИНКЛУДЫ И ЛИНКОВКА
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/includes")

# ЛИНКОВКА - ИСПРАВЛЕНО: использование правильных имен таргетов
target_link_libraries(${PROJECT_NAME} PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    nlohmann_json::nlohmann_json
)

# Дополнительные либы по платформам
if(ANDROID)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "-Wl,--whole-archive" sfml-main "-Wl,--no-whole-archive"
        android
        log
        GLESv2
        EGL
    )
elseif(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE sfml-main)
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework CoreFoundation")
endif()

# КОПИРОВАНИЕ РЕСУРСОВ (Assets)
if(APPLE)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets" "$<TARGET_BUNDLE_CONTENT_DIR:${PROJECT_NAME}>/Resources/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/data" "$<TARGET_BUNDLE_CONTENT_DIR:${PROJECT_NAME}>/Resources/data"
    )
elseif(NOT ANDROID) # На Android Gradle сам забирает ассеты через build.gradle.kts
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/data" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/data"
    )
endif()

# Установка через cmake --install
if(UNIX AND NOT APPLE AND NOT ANDROID)
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)

    # Установка папок с ассетами в /usr/share/tower-defence/
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets" DESTINATION "share/${PROJECT_NAME}")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data"   DESTINATION "share/${PROJECT_NAME}")

    # Установка иконки и ярлыка
    install(FILES "assets/icons/game_icon.png" DESTINATION "share/icons/hicolor/512x512/apps" RENAME "tower-defence.png")
    install(FILES "tower-defence.desktop" DESTINATION "share/applications")
endif()