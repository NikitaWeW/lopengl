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

int main()
{
    Application app;
    GLFWwindow *window = app.window;
    ImGuiIO &io = ImGui::GetIO();
    double deltatime;
    int testIndex = 0;
    const char *testNames[] = {
        "camera test",
        "clear color test"
    };
    std::unique_ptr<test::Test> currentTest = std::make_unique<test::TestCamera>(window); 

    while (!glfwWindowShouldClose(window))
    {
        auto start = std::chrono::high_resolution_clock::now();
        currentTest->onRender(window, deltatime);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        currentTest->onImGuiRender(deltatime);
        ImGui::Begin("properties");
        if(ImGui::Combo("test", &testIndex, testNames, IM_ARRAYSIZE(testNames))) {
            switch (testIndex) {
            case 0:
                currentTest = std::make_unique<test::TestCamera>(window);
                break;
            case 1:
                currentTest = std::make_unique<test::TestClearcolor>();
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
        deltatime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() * 0.001;
    }
}
