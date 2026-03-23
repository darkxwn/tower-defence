#include "Game.hpp"
#include <iostream>
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <unistd.h>
#endif

int main() {
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
    setlocale(LC_ALL, "ru");
    try {
        Game game;
        game.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

	return 0;
}