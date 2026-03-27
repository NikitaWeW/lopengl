#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"

namespace ease
{
    using FuncPtr = float (*)(float);

    constexpr inline float linear(float x)     { return x; }
    constexpr inline float inQuad(float x)     { return x * x; }
    constexpr inline float outQuad(float x)    { return x * (2 - x); }
    constexpr inline float inOutQuad(float x)  { return x < 0.5f ? 2 * x * x : -1 + (4 - 2 * x) * x; }
    constexpr inline float inCubic(float x)    { return x * x * x; }
    constexpr inline float outCubic(float x)   { return glm::pow(x - 1, 3) + 1; }
    constexpr inline float inOutCubic(float x) { return x < 0.5f ? 4 * x * x * x : glm::pow(x - 1, 3) * 4 + 1; }
    constexpr inline float inQuart(float x)    { return glm::pow(x, 4); }
    constexpr inline float outQuart(float x)   { return 1 - glm::pow(x - 1, 4); }
    constexpr inline float inOutQuart(float x) { return x < 0.5f ? 8 * glm::pow(x, 4) : 1 - 8 * glm::pow(x - 1, 4); }
    constexpr inline float inQuint(float x)    { return glm::pow(x, 5); }
    constexpr inline float outQuint(float x)   { return 1 + glm::pow(x - 1, 5); }
    constexpr inline float inOutQuint(float x) { return x < 0.5f ? 16 * glm::pow(x, 5) : 1 + 16 * glm::pow(x - 1, 5); }
    constexpr inline float inSine(float x)     { return 1 - glm::cos(x * glm::half_pi<float>()); }
    constexpr inline float outSine(float x)    { return glm::sin(x * glm::half_pi<float>()); }
    constexpr inline float inOutSine(float x)  { return -(glm::cos(glm::pi<float>() * x) - 1) * 0.5f; }
    constexpr inline float inExpo(float x)     { return x == 0 ? 0 : glm::pow(2, 10 * (x - 1)); }
    constexpr inline float outExpo(float x)    { return x == 1 ? 1 : 1 - glm::pow(2, -10 * x); }
    constexpr inline float inOutExpo(float x)  { return x == 0 ? 0 : x == 1 ? 1 : x < 0.5f ? glm::pow(2, 20 * x - 10) * 0.5f : (2 - glm::pow(2, -20 * x + 10)) * 0.5f; }
    constexpr inline float inCirc(float x)     { return 1 - glm::sqrt(1 - glm::pow(x, 2)); }
    constexpr inline float outCirc(float x)    { return glm::sqrt(1 - glm::pow(x - 1, 2)); }
    constexpr inline float inOutCirc(float x)  { return x < 0.5f ? (1 - glm::sqrt(1 - 4 * x * x)) * 0.5f : (glm::sqrt(1 - glm::pow(x - 1, 2)) + 1) * 0.5f; }
} // namespace ease
