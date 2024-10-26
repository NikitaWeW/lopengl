#include "opengl/GlCall.h"
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"

#include "Application.hpp"
#include "test/TestCamera.hpp"
#include "test/TestClearColor.hpp"

#include <chrono>
#include <memory>

#ifdef NDEBUG
extern const bool debug = false;
#else
extern const bool debug = true;
#endif

float GetTime() {
    typedef std::chrono::high_resolution_clock clock;
    typedef std::chrono::duration<float, std::milli> duration;

    return clock::now().time_since_epoch().count();
}
int main()
{
    Application app;
    GLFWwindow *window = app.window;
    ImGuiIO &io = ImGui::GetIO();
    double deltatime;
    int testIndex = 0;
    const char *testNames[] = {
        "clear color test",
        "camera test",
        "texture test"
    };
    std::unique_ptr<test::Test> currentTest = std::make_unique<test::TestClearColor>(window); 

    while (!glfwWindowShouldClose(window))
    {
        auto start = GetTime();
        currentTest->onRender(window, deltatime);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        currentTest->onImGuiRender(deltatime);
        ImGui::Begin("properties");
        if(ImGui::Combo("test", &testIndex, testNames, IM_ARRAYSIZE(testNames))) {
            switch (testIndex) {
            case 0:
                currentTest = std::make_unique<test::TestClearColor>();
                break;
            case 1:
                currentTest = std::make_unique<test::TestCamera>(window);
                break;
            case 2:
                currentTest = std::make_unique<test::TestClearColor>();
                break;
            default:
                break;
            }
        }
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
        deltatime = GetTime() - start;
    }
}
