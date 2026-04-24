#pragma once
#include <SFML/Graphics.hpp>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС TILE
//
///////////////////////////////////////////////////////////////////////////

// Типы тайлов
enum class TileType {
	Empty, // пустой тайл
	Portal, // тайл портала
	Road, // тайл дороги
	Base, // тайл базы
	Platform // тайл платформы
};

// Структура тайла
struct Tile {
	TileType type = TileType::Empty; // тип тайла
	sf::Vector2i gridPos; // позиция на сетке
	const sf::Texture* texture = nullptr; // кэшированный указатель на текстуру
};