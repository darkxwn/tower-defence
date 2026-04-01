cmake_minimum_required(VERSION 3.15)

set(CMAKE_POLICY_DEFAULT_CMP0000 NEW)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

project(tower-defence LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
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

if(ANDROID)
    message(STATUS "Configuring for Android...")
    
    # SFML 3 на Android требует статическую сборку некоторых компонентов 
    # или специфическую линковку sfml-main
    add_definitions(-DANDROID)
    
    # Если ты используешь FetchContent, SFML сам поймет, что он на Android,
    # но нам нужно линковать системные библиотеки
    list(APPEND ANDROID_LIBS android log GLESv2 EGL)

    # Указываем CMake класть готовую библиотеку в папку jniLibs для Android
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/android/app/src/main/jniLibs/${ANDROID_ABI}"
    )
endif()

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/includes"
)

if(ANDROID)
    target_link_libraries(${PROJECT_NAME} PRIVATE
            SFML::Graphics
            SFML::Window
            SFML::System
            # МАГИЯ ЗДЕСЬ: заставляем включить sfml-main целиком
            "-Wl,--whole-archive" SFML::Main "-Wl,--no-whole-archive"
        
            # Системные либы андроида
            android
            log
            GLESv2
            EGL
        )
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        SFML::Graphics
        SFML::Window
        SFML::System
        SFML::Main
        # SFML::Audio
    )
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