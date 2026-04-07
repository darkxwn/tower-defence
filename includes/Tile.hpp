#pragma once
#include <SFML/Graphics.hpp>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС TILE
//
///////////////////////////////////////////////////////////////////////////

enum class TileType {
	Empty,           // 0 Тайл фона
	Portal,          // 1 Тайл портала - место выхода врагов
	Road,            // 2 Тайл дороги
	Base,            // 3 Тайл базы
	Platform         // 4 Тайл платформы
};

struct Tile {
	TileType type = TileType::Empty;          // Тип тайла
	sf::Vector2i gridPos;   // Позиция на карте
};