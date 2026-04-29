#pragma once
#include <SFML/Graphics.hpp>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС TILE
//
///////////////////////////////////////////////////////////////////////////

// Типы тайлов
enum class TileType {
	Empty,   // 0 пустой тайл
	Portal,  // 1 тайл портала
	Road,    // 2 тайл дороги
	Base,    // 3 тайл базы
	Platform // 4 тайл платформы
};

// Структура тайла
struct Tile {
	TileType type = TileType::Empty; // тип тайла
	sf::Vector2i gridPos; // позиция на сетке
	const sf::Texture* texture = nullptr; // кэшированный указатель на текстуру
};