#include "VFXManager.hpp"
#include <cmath>
#include <cstdlib>

namespace Engine {

    VFXManager::VFXManager() {
        // использование треугольников (2 на частицу, 6 вершин) для совместимости
        vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
        particles.reserve(2000); // резервирование памяти для предотвращения частых аллокаций
    }

    void VFXManager::createHitEffect(sf::Vector2f pos, sf::Color color, int count) {
        for (int i = 0; i < count; ++i) {
            Particle p;
            p.position = pos;

            float angle = static_cast<float>(std::rand() % 360) * 3.14159f / 180.f;

            // УМЕНЬШЕНО: начальная скорость (теперь 40-100 вместо 60-180)
            float speed = static_cast<float>(std::rand() % 250 + 150);

            p.velocity = { std::cos(angle) * speed, std::sin(angle) * speed };

            p.color = sf::Color(
                std::min(color.r + 50, 255),
                std::min(color.g + 50, 255),
                std::min(color.b + 50, 255)
            );

            // УМЕНЬШЕНО: время жизни (0.25 - 0.45 сек), чтобы не успевали улетать далеко
            p.maxLifetime = 0.25f + static_cast<float>(std::rand() % 20) / 100.f;
            p.lifetime = p.maxLifetime;

            particles.push_back(p);
        }
    }

    // Создание частицы следа снаряда
    void VFXManager::createTrailUnit(sf::Vector2f pos, sf::Color color) {
        Particle p;
        float rx = (std::rand() % 4 - 2);
        float ry = (std::rand() % 4 - 2);
        p.position = pos + sf::Vector2f(rx, ry);
        p.velocity = { 0.f, 0.f };
        p.color = color;
        p.maxLifetime = 0.3f + static_cast<float>(std::rand() % 20) / 100.f;
        p.lifetime = p.maxLifetime;

        particles.push_back(p);
    }

    // Усиленный взрыв при смерти
    void VFXManager::createDeathExplosion(sf::Vector2f pos, sf::Color color) {
        // Увеличиваем количество частиц для "финального" аккорда
        createHitEffect(pos, color, 25);

        // Можно добавить несколько белых частиц для яркости вспышки
        createHitEffect(pos, sf::Color::White, 5);
    }

    void VFXManager::update(float deltaTime) {
        if (particles.empty()) return;

        for (auto& p : particles) {
            p.lifetime -= deltaTime;
            p.position += p.velocity * deltaTime;
            p.velocity *= 0.90f;
        }

        particles.erase(
            std::remove_if(particles.begin(), particles.end(),
                [](const Particle& p) { return p.lifetime <= 0.0f; }),
            particles.end()
        );
    }

    void VFXManager::render(sf::RenderTarget& target) {
        if (particles.empty()) return;

        vertices.resize(particles.size() * 6);

        for (size_t i = 0; i < particles.size(); ++i) {
            const auto& p = particles[i];
            float lifeRatio = p.lifetime / p.maxLifetime;
            if (lifeRatio < 0) lifeRatio = 0;

            sf::Color c = p.color;
            c.a = static_cast<uint8_t>(255 * lifeRatio);

            float speed = std::sqrt(p.velocity.x * p.velocity.x + p.velocity.y * p.velocity.y);
            size_t idx = i * 6;

            // ЛОГИКА ТРАССЕРА (статичная частица)
            if (speed < 0.1f) {
                float size = 4.0f * lifeRatio; // размер следа
                setQuad(idx, p.position, size, c);
            }
            // ЛОГИКА ИСКРЫ (движущаяся частица)
            else {
                sf::Vector2f unitVel = p.velocity / speed;
                sf::Vector2f normal = { -unitVel.y, unitVel.x };

                float length = 12.0f * lifeRatio * (speed / 200.f);
                float width = 1.5f;

                sf::Vector2f head = p.position + unitVel * length;
                sf::Vector2f tail = p.position;

                vertices[idx + 0] = { tail - normal * width, c };
                vertices[idx + 1] = { head - normal * width, c };
                vertices[idx + 2] = { head + normal * width, c };
                vertices[idx + 3] = { head + normal * width, c };
                vertices[idx + 4] = { tail + normal * width, c };
                vertices[idx + 5] = { tail - normal * width, c };
            }
        }

        target.draw(vertices);
    }

    void VFXManager::setQuad(size_t index, sf::Vector2f pos, float size, sf::Color color) {
        float h = size / 2.f;

        // Первый треугольник
        vertices[index + 0] = { {pos.x - h, pos.y - h}, color };
        vertices[index + 1] = { {pos.x + h, pos.y - h}, color };
        vertices[index + 2] = { {pos.x - h, pos.y + h}, color };

        // Второй треугольник
        vertices[index + 3] = { {pos.x + h, pos.y - h}, color };
        vertices[index + 4] = { {pos.x + h, pos.y + h}, color };
        vertices[index + 5] = { {pos.x - h, pos.y + h}, color };
    }

    void VFXManager::clear() {
        particles.clear();
        vertices.clear();
    }

} // namespace Engine