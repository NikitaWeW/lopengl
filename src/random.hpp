#pragma once
#include <random>
#include "GLFW/glfw3.h"

template <typename T>
T randRange(T min, T max) {
    static_assert(false && "not supported");
}

template <>
float randRange(float min, float max) {
    static std::mt19937 rng(std::random_device{}());  
    auto x = std::generate_canonical<float, 128>(rng) * (max - min) + min;
    return x;
}