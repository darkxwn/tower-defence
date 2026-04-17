#pragma once
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <random>

namespace Math {
    inline constexpr float PI = 3.1415926535f;

    // ГЕОМЕТРИЯ
    template <typename T>
    inline T getLengthSq(const sf::Vector2<T>& v) { return v.x * v.x + v.y * v.y; }

    template <typename T>
    inline float getLength(const sf::Vector2<T>& v) { return std::sqrt(static_cast<float>(getLengthSq(v))); }

    template <typename T>
    inline T getDistSq(const sf::Vector2<T>& p1, const sf::Vector2<T>& p2) { return getLengthSq(p1 - p2); }

    template <typename T>
    inline float getDist(const sf::Vector2<T>& p1, const sf::Vector2<T>& p2) { return std::sqrt(static_cast<float>(getDistSq(p1, p2))); }

    inline sf::Vector2f normalize(const sf::Vector2f& v) {
        float len = getLength(v);
        return (len < 0.0001f) ? sf::Vector2f(0.f, 0.f) : v / len;
    }

    inline float radToDeg(float rad) { return rad * 180.f / PI; }

    // Получение угла вектора для SFML (0 градусов - это вверх)
    inline float getAngle(const sf::Vector2f& v) {
        return radToDeg(std::atan2(v.y, v.x)) + 90.f;
    }

    // РАНДОМ
    namespace Random {
        inline std::mt19937& getEngine() {
            static std::mt19937 engine{ std::random_device{}() };
            return engine;
        }

        inline int getInt(int min, int max) {
            return std::uniform_int_distribution<int>(min, max)(getEngine());
        }

        inline float getFloat(float min, float max) {
            return std::uniform_real_distribution<float>(min, max)(getEngine());
        }

        inline bool getChance(float p) {
            return getFloat(0.f, 1.f) < p;
        }
    }
}