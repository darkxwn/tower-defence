#pragma once
#include <map>
#include <vector>
#include <string>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС GAMEDATA
// Хранилище статических данных игры: характеристики врагов и башен.
//
///////////////////////////////////////////////////////////////////////////

// Характеристики врага
struct EnemyStats {
	int health; // количество жизней
	int speed;  // скорость передвижения
	int damage; // урон по базе
	int reward; // награда за уничтожение
};

// Характеристики башни
struct TowerStats {
	int damage;     // урон одного выстрела
	float range;    // радиус поражения
	float firerate; // частота стрельбы
	int cost;       // стоимость постройки
	int splash;     // радиус взрыва (0 если нет)
	int level;      // уровень башни
	int costDamage;      // цена улучшения урона
	int costFirerate;   // цена улучшения скорости стрельбы
	int costRange;      // цена улучшения дальности
	int costLevel;     // цена повышения уровня
};

class GameData {
private:
	static std::map<std::string, EnemyStats> enemies; // хранилище врагов (ключ - имя типа)
	static std::map<std::string, TowerStats> towers;  // хранилище башен (ключ - имя типа)
	static std::vector<std::string> towerOrder;       // очередность башен в магазине
	static std::vector<std::string> enemyTypes;       // список всех доступных типов врагов

public:
	// Загрузка всех данных из .cfg файлов
	static void load();

	// Получение характеристик врага по имени типа
	static EnemyStats getEnemy(const std::string& type);

	// Получение характеристик башни по имени
	static TowerStats getTower(const std::string& name);

	// Получение списка всех доступных башен
	static std::vector<std::string> getTowerNames();

	// Получение списка всех типов врагов, найденных в конфигурации
	static std::vector<std::string> getEnemyTypes();

	// Получение базовых характеристик башни для инициализации улучшений
	static TowerStats getBaseTowerStats(const std::string& name);
};
