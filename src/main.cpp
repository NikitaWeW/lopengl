/*
        +____________+
        /:\         ,:\
       / : \       , : \
      /  :  \     ,  :  \
     /   :   +-----------+
    +....:../:...+   :  /|
    |\   +./.:...`...+ / |
    | \ ,`/  :   :` ,`/  |
    |  \ /`. :   : ` /`  |
    | , +-----------+  ` |
    |,  |   `+...:,.|...`+
    +...|...,'...+  |   /
     \  |  ,     `  |  /
      \ | ,       ` | /
       \|,         `|/
        +___________+

2-Dimensional ASCII Representation Of A 3-Dimensional Cross-Section Of A 4-Dimensional Cube

---

Copyright (c) 2025 Nikita Martynau (https://opensource.org/license/mit)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "main.hpp"

int main(int argc, char **argv)
{
    Data data{};

    if(!init(data)) {
        std::cout << "failed to init!\n";
        return -1;
    }

    // ===================================

    data.cubeDrawShader     = ogl::ShaderProgram{"shaders/prop"};
    data.cubeGenerateShader = ogl::ShaderProgram{"shaders/generateTexture"};
    data.displayShader      = ogl::ShaderProgram{"shaders/hdrImage"};
    data.skyboxDrawShader   = ogl::ShaderProgram{"shaders/skybox"};
    data.gridShader         = ogl::ShaderProgram{"shaders/grid"};

    data.mainFBO    = ogl::Framebuffer{0};
    data.displayFBO = ogl::Framebuffer{0};

    data.mainRBO    = ogl::Renderbuffer{0};
    data.displayRBO = ogl::Renderbuffer{0};

    data.displayTexture = ogl::Texture{GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE};

    data.skybox = ogl::Cubemap{"res/textures/space.jpg"};
    model::Loader loader;
    data.cube = loader.load("res/models/cube.obj");
    
    data.distance.falloff = 10;
    data.inputs.sensitivity = 0.5;

    randomSeed(data);

    // ===================================
    
    glfwSetWindowUserPointer(data.window, &data);
    glfwSetScrollCallback(data.window, scroll_callback);
    glfwSetKeyCallback(data.window, key_callback);

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(data.window))
    {
        auto start = std::chrono::high_resolution_clock::now();

        drawFrame(data);
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuiIO &io = ImGui::GetIO();
        glViewport(0, 0, data.windowSize.x, data.windowSize.y);
        if(io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("Editor DockSpace");
            ImGui::DockSpaceOverViewport(dockspace_id, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
        }
                

        ui(data);

        glfwPollEvents();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
        glfwSwapBuffers(data.window);
        data.deltatime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() * 1.0E-6;
    }
    
    glfwDestroyWindow(data.window);
    glfwTerminate();
}
