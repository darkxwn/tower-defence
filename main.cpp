#include "Game.hpp"
#include <iostream>

int main() {
    setlocale(LC_ALL, "ru");
    try {
        Game game;
        game.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        system("pause");
    }

	return 0;
}