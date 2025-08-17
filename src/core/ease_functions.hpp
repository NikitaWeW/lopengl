#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"

namespace ease
{
    template<typename T = float>
    using easeFuncPtr = T (*)(T);

    template<typename T = float> constexpr inline T liniar(T x)     { return x; }
    template<typename T = float> constexpr inline T inQuad(T x)     { return x * x; }
    template<typename T = float> constexpr inline T outQuad(T x)    { return x * (T{2} - x); }
    template<typename T = float> constexpr inline T inOutQuad(T x)  { return x < T{0.5f} ? T{2} * x * x : -T{1} + (T{4} - T{2} * x) * x; }
    template<typename T = float> constexpr inline T inCubic(T x)    { return x * x * x; }
    template<typename T = float> constexpr inline T outCubic(T x)   { return glm::pow(x - T{1}, T{3}) + T{1}; }
    template<typename T = float> constexpr inline T inOutCubic(T x) { return x < T{0.5f} ? T{4} * x * x * x : glm::pow(x - T{1}, T{3}) * T{4} + T{1}; }
    template<typename T = float> constexpr inline T inQuart(T x)    { return glm::pow(x, T{4}); }
    template<typename T = float> constexpr inline T outQuart(T x)   { return T{1} - glm::pow(x - T{1}, T{4}); }
    template<typename T = float> constexpr inline T inOutQuart(T x) { return x < T{0.5f} ? T{8} * glm::pow(x, T{4}) : T{1} - T{8} * glm::pow(x - T{1}, T{4}); }
    template<typename T = float> constexpr inline T inQuint(T x)    { return glm::pow(x, T{5}); }
    template<typename T = float> constexpr inline T outQuint(T x)   { return T{1} + glm::pow(x - T{1}, T{5}); }
    template<typename T = float> constexpr inline T inOutQuint(T x) { return x < T{0.5f} ? T{16} * glm::pow(x, T{5}) : T{1} + T{16} * glm::pow(x - T{1}, T{5}); }
    template<typename T = float> constexpr inline T inSine(T x)     { return T{1} - glm::cos(x * T{glm::half_pi<float>()}); }
    template<typename T = float> constexpr inline T outSine(T x)    { return glm::sin(x * T{glm::half_pi<float>()}); }
    template<typename T = float> constexpr inline T inOutSine(T x)  { return -(glm::cos(T{glm::pi<float>()} * x) - T{1}) * T{0.5f}; }
    template<typename T = float> constexpr inline T inExpo(T x)     { return x == T{0} ? T{0} : glm::pow(T{2}, T{10} * (x - T{1})); }
    template<typename T = float> constexpr inline T outExpo(T x)    { return x == T{1} ? T{1} : T{1} - glm::pow(T{2}, -T{10} * x); }
    template<typename T = float> constexpr inline T inOutExpo(T x)  { return x == T{0} ? T{0} : x == T{1} ? T{1} : x < T{0.5f} ? glm::pow(T{2}, T{20} * x - T{10}) * T{0.5f} : (T{2} - glm::pow(T{2}, T{-20} * x + T{10})) * T{0.5f}; }
    template<typename T = float> constexpr inline T inCirc(T x)     { return T{1} - glm::sqrt(T{1} - glm::pow(x, T{2})); }
    template<typename T = float> constexpr inline T outCirc(T x)    { return glm::sqrt(T{1} - glm::pow(x - T{1}, T{2})); }
    template<typename T = float> constexpr inline T inOutCirc(T x)  { return x < T{0.5f} ? (T{1} - glm::sqrt(T{1} - T{4} * x * x)) * T{0.5f} : (glm::sqrt(T{1} - glm::pow(x - T{1}, T{2})) + T{1}) * T{0.5f}; }
} // namespace ease
