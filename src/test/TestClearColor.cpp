#include "opengl/GlCall.h"
#include "TestClearColor.hpp"
#include "imgui.h"

test::TestClearColor::TestClearColor() : color({0.05, 0.1, 0.12}) {
    LOG_INFO("creating test");
}
test::TestClearColor::~TestClearColor() {
    LOG_INFO("destroying test");
}
void test::TestClearColor::onRender(GLFWwindow *window, double deltatime)
{
    GLCALL(glClearColor(color[0], color[1], color[2], color[3]));
    GLCALL(glClear(GL_COLOR_BUFFER_BIT));
}
void test::TestClearColor::onImGuiRender(double deltatime)
{
    ImGui::Begin("clear color test");
    ImGui::ColorEdit4("color", color.data());
    ImGui::End();
}