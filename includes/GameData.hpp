#pragma once
#include "Enemy.hpp"
#include <map>
#include <vector>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС GAMEDATA
//
///////////////////////////////////////////////////////////////////////////

// Статы врага
struct EnemyStats {
	int health; // количество жизней
	int speed; // скорость
	int damage; // урон по базе
	int reward; // награда за убийство
};

// Статы башни
struct TowerStats {
	int damage; // урон
	float range; // радиус атаки
	float firerate; // скорострельность
	int cost; // цена постройки
	int splash; // урон по области
};

class GameData {
private:
	static std::map<EnemyType, EnemyStats> enemies; // хранилище статов врагов
	static std::map<std::string, TowerStats> towers; // хранилище статов башен
	static std::vector<std::string> towerOrder; // порядок башен

public:
	// Загрузка данных из конфигурационных файлов
	static void load();

	// Получение статов врага по типу
	static EnemyStats getEnemy(EnemyType type);

	// Получение статов башни по имени
	static TowerStats getTower(const std::string& name);

	// Получение имён всех башен
	static std::vector<std::string> getTowerNames();
};
