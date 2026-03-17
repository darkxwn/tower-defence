#pragma once
#include <vector>
#include <string>
#include "Tile.hpp"

// Класс карта
class Map {
private:
	int width = 0;                        // Ширина карты в тайлах
	int height = 0;                       // Высота карты в тайлах
	int startMoney = 0;                   // Стартовое количество денег

	std::vector<std::vector<Tile>> tiles; // Карта
	Tile* selectedTile = nullptr;         // Указатель на текущий выбранный тайл

	std::vector<sf::Vector2i> path;       // Массив клеток пути движения врагов
	sf::Vector2i portalPos;               // Координаты Portal
	sf::Vector2i basePos;                 // Координаты Base

	sf::Vector2f mapOffset;               // Смещение карты на экране

	void buildPath();

public:
	Map();
	void load(const std::string& path);
	void render(sf::RenderWindow& window);

	void centerOnScreen(sf::Vector2u windowSize, float topPanelHeight, float bottomPanelHeight);
	
	void setSelectedTile(sf::Vector2f screenPos);
	Tile* getTileAtScreen(sf::Vector2f screenPos) const;

	const std::vector<sf::Vector2i>& getPath() const;
	sf::Vector2i getBasePos() const;
	int getStartMoney() const;
	sf::Vector2f getMapOffset() const;
	
};