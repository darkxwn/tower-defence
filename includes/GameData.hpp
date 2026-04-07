#pragma once
#include "Enemy.hpp"
#include <map>
#include <vector>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС GAMEDATA
//
///////////////////////////////////////////////////////////////////////////

// Структура статов врага
struct EnemyStats {
	int health;      // Количество жизней 
	int speed;       // Скорость 
	int damage;      // Урон по базе
	int reward;      // Награда за убийство
};

// Структура статов башни
struct TowerStats {
	int damage;      // Урон
	float range;     // Радиус атаки
	float firerate;  // Скорострельность
	int cost;        // Цена постройки
	int splash;      // Урон по области 
};

class GameData {
private:
	static std::map<EnemyType, EnemyStats>  enemies;
	static std::map<std::string, TowerStats> towers;
	static std::vector<std::string> towerOrder;

public:
	static void load();
	static EnemyStats getEnemy(EnemyType type);
	static TowerStats getTower(const std::string& name);
	static std::vector<std::string> getTowerNames();
};