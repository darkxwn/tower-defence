#include <SFML/Graphics.hpp>

int main() {
	setlocale(LC_ALL, "ru");

	sf::RenderWindow window(sf::VideoMode({ 1280, 720 }), "Tower Defence"); 
	window.setFramerateLimit(60);
    
    while (window.isOpen()) {
        while (std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }
        window.clear(sf::Color(27, 27, 27));
        window.display();
    }


	return 0;
}