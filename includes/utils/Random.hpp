#pragma once
#include <random>

class Random {
    private:
        inline static std::mt19937 engine{ std::random_device{}() };
    
    public:
        static int getInt(int min, int max) {
            std::uniform_int_distribution<int> dist(min, max);
            return dist(engine);
        }

        static float getFloat(float min, float max) {
            std::uniform_real_distribution<float> dist(min, max);
            return dist(engine);
        }
};