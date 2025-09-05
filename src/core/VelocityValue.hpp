#pragma once
#include "ease_functions.hpp"

template <typename T>
struct VelocityValue
{
    ease::easeFuncPtr<T> easeFunc = ease::outCirc<T>;
    T velocity = T{0};
    T value = T{0};
    T falloff = T{1};
    glm::vec2 edges{0.0f, 10.0f};

    inline void update(float deltatime)
    {
        value += velocity * deltatime;
        T x = glm::clamp((glm::abs(velocity) - T{edges.x}) / (T{edges.y} - T{edges.x}), T{0}, T{1});
        T curve = easeFunc(x);
        velocity -= velocity * curve * deltatime * falloff;
    }
};