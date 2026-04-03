cmake_minimum_required(VERSION 3.15)

set(CMAKE_POLICY_DEFAULT_CMP0000 NEW)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

project(tower-defence LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SFML_STATIC_LIBRARIES ON CACHE BOOL "" FORCE)

set(MACOS_ICON "${CMAKE_CURRENT_SOURCE_DIR}/game_macos.icns")

include(FetchContent)
FetchContent_Declare(sfml
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW TRUE 
)

# Настройки сборки SFML, чтобы не тратить время на лишнее
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)
set(SFML_INSTALL_PKGCONFIG_FILES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(sfml)

if(UNIX AND NOT APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()

file(GLOB_RECURSE SOURCES "src/*.cpp")
list(APPEND SOURCES "main.cpp")

# 1. Ищем все файлы .map в папке уровней
file(GLOB MAP_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/data/levels" "data/levels/*.map")

# 2. Превращаем список файлов в строку через запятую для C++ (напр. "level01.map", "level02.map")
list(JOIN MAP_FILES "\", \"" MAP_STR)
set(MAP_STR "\"${MAP_STR}\"")

# 3. Генерируем мини-хедер прямо в папку сборки
file(WRITE "${CMAKE_BINARY_DIR}/GeneratedLevels.hpp" 
     "#pragma once\n#include <vector>\n#include <string>\n"
     "inline std::vector<std::string> getLevelList() { return { ${MAP_STR} }; }")

# 4. Даем проекту доступ к этому файлу
include_directories("${CMAKE_BINARY_DIR}")

if(ANDROID)
    # Android требует SHARED библиотеку (.so), а не EXECUTABLE
    add_library(${PROJECT_NAME} SHARED ${SOURCES} ${EXE_ICON})
else()
    add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${SOURCES} ${EXE_ICON} ${MACOS_ICON})
endif()


if(APPLE)
    set_source_files_properties(${MACOS_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "Tower Defence"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.darkxwn.tower-defence"
        MACOSX_BUNDLE_BUNDLE_VERSION "0.0.1"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
        MACOSX_BUNDLE_ICON "game_macos.icns"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/macos/Info.plist.in"
    )
endif()

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/includes"
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    SFML::Graphics
    SFML::Window
    SFML::System
)

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
endif()

if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework CoreFoundation")
endif()

if(ANDROID)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${ANDROID_LIBS})
endif()

if(APPLE)
    # На macOS ресурсы кладем внутрь .app пакета
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets" "$<TARGET_BUNDLE_CONTENT_DIR:${PROJECT_NAME}>/Resources/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/data" "$<TARGET_BUNDLE_CONTENT_DIR:${PROJECT_NAME}>/Resources/data"
    )
else()
    # Код для остальных систем (Windows/Linux) 
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/data" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/data"
    )
endif()