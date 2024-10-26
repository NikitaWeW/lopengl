#pragma once
#include "GLFW/glfw3.h"
namespace test {
    class Test
    {
    private:
    public:
        Test() {}
        virtual ~Test() {}
        virtual void onRender(GLFWwindow *window, double deltatime) {}
        virtual void onImGuiRender(double deltatime) {}
    };
    float const squareVertices[] = {
    //  coordinate              texture
        -1.0f, -1.0f,  0.0f,    0.0f, 0.0f, // 0
        1.0f, -1.0f,  0.0f,    1.0f, 0.0f, // 1
        1.0f,  1.0f,  0.0f,    1.0f, 1.0f, // 2
        -1.0f,  1.0f,  0.0f,    0.0f, 1.0f  // 3
    };
    unsigned const squareIndicies[] = {
        0, 1, 2,
        0, 2, 3
    };
}