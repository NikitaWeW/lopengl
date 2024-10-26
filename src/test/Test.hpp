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
}