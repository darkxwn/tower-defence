#ifndef VFXMANAGER_HPP
#define VFXMANAGER_HPP

#include <SFML/Graphics.hpp>
#include <vector>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС VFXMANAGER
//
///////////////////////////////////////////////////////////////////////////

namespace Engine {

    // Одиночная частица эффекта
    struct Particle {
        sf::Vector2f position;
        sf::Vector2f velocity;
        sf::Color color;
        float lifetime;    // текущее время жизни
        float maxLifetime; // начальное время жизни
    };

    class VFXManager {
    public:
        VFXManager();

        // Создание эффекта взрыва частиц при попадании
        void createHitEffect(sf::Vector2f pos, sf::Color color, int count = 8);

        // Создание частицы следа (короткое время жизни, без начальной скорости)
        void createTrailUnit(sf::Vector2f pos, sf::Color color);

        // Усиленный эффект для смерти врага
        void createDeathExplosion(sf::Vector2f pos, sf::Color color);

        // Обновление физики и времени жизни частиц
        void update(float deltaTime);

        // Отрисовка всех частиц через VertexArray
        void render(sf::RenderTarget& target);

        // Очистка всех активных эффектов
        void clear();

    private:
        std::vector<Particle> particles;
        sf::VertexArray vertices; // массив для пакетной отрисовки

        // Настройка геометрии для одной частицы в массиве
        void setQuad(size_t index, sf::Vector2f pos, float size, sf::Color color);
    };

} // namespace Engine

#endif // VFXMANAGER_HPP