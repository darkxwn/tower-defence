cmake_minimum_required(VERSION 3.15...3.30)

set(CMAKE_POLICY_DEFAULT_CMP0000 NEW)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

project(tower-defence LANGUAGES CXX)

if(ANDROID)
    # Принудительно выставляем стандарт для ВСЕХ, включая FetchContent
    set(CMAKE_CXX_STANDARD 20 CACHE STRING "" FORCE)
    set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "" FORCE)
    
    # Глобальный хак: заставляем компилятор и линковщик использовать c++_shared
    # Эти флаги добавятся К КАЖДОЙ команде компиляции и линковки
    add_compile_options(-stdlib=libc++)
    add_link_options("-static-libstdc++") 
    
    # Принудительно задаем STL для NDK Toolchain
    set(CMAKE_ANDROID_STL_TYPE c++_shared CACHE STRING "" FORCE)
    set(ANDROID_STL c++_shared CACHE STRING "" FORCE)
    
    set(SFML_OS_ANDROID ON CACHE BOOL "" FORCE)
    set(SFML_USE_SYSTEM_DEPS OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_MAIN ON CACHE BOOL "" FORCE)
    link_libraries(c++_shared)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Настройки статической сборки
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SFML_STATIC_LIBRARIES ON CACHE BOOL "" FORCE)

include(FetchContent)

# JSON
FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_SHALLOW ON
    GIT_TAG v3.12.0
)

# SFML
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)

# Оптимизация сборки SFML
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)
set(SFML_INSTALL_PKGCONFIG_FILES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(sfml json)

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
    src/utils/FileReader.cpp
    src/utils/Logger.cpp
    src/ui/Text.cpp
    src/ui/Button.cpp
    src/ui/Widget.cpp
    src/ui/Container.cpp
    src/ui/Slider.cpp
    src/ui/Image.cpp
)

# Ресурсы для Windows и macOS
set(WIN_RESOURCES "")
if(WIN32)
    # Если у тебя есть файл .rc для иконки Windows, добавь его сюда
    # set(WIN_RESOURCES "src/resource.rc")
endif()

set(MACOS_ICON "${CMAKE_CURRENT_SOURCE_DIR}/game_macos.icns")

# СОЗДАНИЕ ЦЕЛИ (Target)
if(ANDROID)
    # Android требует библиотеку .so
    add_library(${PROJECT_NAME} SHARED ${SOURCES})
else()
    # Остальные - исполняемый файл
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
        MACOSX_BUNDLE_BUNDLE_NAME "Tower Defence"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.darkxwn.tower-defence"
        MACOSX_BUNDLE_BUNDLE_VERSION "0.0.1"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
        MACOSX_BUNDLE_ICON_FILE "game_macos.icns"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/macos/Info.plist.in"
    )
endif()

if(WIN32)
    # Точка входа для графического приложения (отключает консоль)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
endif()

# ИНКЛУДЫ И ЛИНКОВКА
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/includes")

# Общие библиотеки
target_link_libraries(${PROJECT_NAME} PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    #SFML::Audio
    nlohmann_json::nlohmann_json
)

# Дополнительные либы по платформам
if(ANDROID)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "-Wl,--whole-archive" SFML::Main "-Wl,--no-whole-archive"
        android
        log
        GLESv2
        EGL
    )
elseif(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE SFML::Main)
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
