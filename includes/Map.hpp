#pragma once
#include <vector>
#include <string>
#include "Tile.hpp"

// Класс карта
class Map {
private:
	int width = 0;                        // Ширина карты в тайлах
	int height = 0;                       // Высота карты в тайлах
	int money = 0;                        // Текущее количество денег на уровне

	std::vector<std::vector<Tile>> tiles; // Карта
	Tile* selectedTile = nullptr;         // Указатель на текущий выбранный тайл

	std::vector<sf::Vector2i> path;       // Массив клеток пути движения врагов
	sf::Vector2i portalPos;               // Координаты Portal
	sf::Vector2i basePos;                 // Координаты Base

	void buildPath();

public:
	Map();
	void load(const std::string& path);
	void render(sf::RenderWindow& window);

	const std::vector<sf::Vector2i>& getPath() const;
	sf::Vector2i getBasePos() const;
};