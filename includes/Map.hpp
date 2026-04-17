#pragma once
#include <vector>
#include <string>
#include "Tile.hpp"

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС MAP
//
///////////////////////////////////////////////////////////////////////////

class Map {
private:
    int width = 0; // ширина карты в тайлах
    int height = 0; // высота карты в тайлах
    int startMoney = 0; // стартовые деньги

    std::vector<std::vector<Tile>> tiles; // сетка тайлов
    Tile* selectedTile = nullptr; // выбранный тайл

    float portalAngle = 0.f; // угол вращения слоя портала

    std::vector<sf::Vector2i> path; // путь врагов
    sf::Vector2i portalPos; // позиция портала
    sf::Vector2i basePos; // позиция базы
    sf::Vector2f mapOffset; // смещение карты

    // Построение пути от портала к базе
    void buildPath();

public:
    // Загрузка уровня из .map файла
    void load(const std::string& filePath);

    // Обновление анимации портала
    void update(float dt);

    // Отрисовка карты
    void render(sf::RenderWindow& window);

    // Центрирование карты на экране
    void centerOnScreen(sf::Vector2u windowSize, float topPanelH, float bottomPanelH);

    // Изменение выбранного тайла по экранным координатам
    void setSelectedTile(sf::Vector2f screenPos);

    // Получение тайла по экранным координатам
    Tile* getTileAtScreen(sf::Vector2f screenPos) const;

    // Получение выбранного тайла
    Tile* getSelectedTile() const;

    // Получение пути врагов
    const std::vector<sf::Vector2i>& getPath() const;

    // Получение позиции базы
    sf::Vector2i getBasePos() const;

    // Получение стартовых денег
    int getStartMoney() const;

    // Получение смещения карты
    sf::Vector2f getMapOffset() const;

    // Получение ширины карты
    int getWidth() const;

    // Получение высоты карты
    int getHeight() const;
};
