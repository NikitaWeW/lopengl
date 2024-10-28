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
    double deltatime = 0;
    double imguideltatime = 0;
    int testIndex = 0;
    unsigned frameCounter = 0;
    const char *testNames[] = {
        "camera test",
        "clear color test"
    };
    std::unique_ptr<test::Test> currentTest = std::make_unique<test::TestCamera>(window); 

    while (!glfwWindowShouldClose(window))
    {
        auto start = std::chrono::high_resolution_clock::now();
        
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
        ImGui::Text("delta time: %f", deltatime);
        ImGui::Text("FPS: %f", deltatime ? 1/deltatime : -1);
        ImGui::Separator();
        ImGui::Text("imgui delta time: %f", imguideltatime);
        ImGui::Text("delta time without gui: %f", deltatime - imguideltatime);
        ImGui::Text("FPS without gui: %f", (deltatime - imguideltatime) ? 1/(deltatime - imguideltatime) : -1);
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
        imguideltatime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() * 0.000001;

        currentTest->onRender(window, deltatime);

        glfwSwapBuffers(window);
        glfwPollEvents();
        ++frameCounter;
        deltatime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() * 0.000001;
    }
}
