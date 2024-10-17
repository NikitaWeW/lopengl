#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Application.hpp"
#include "Renderer.hpp"
#include "Shader.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include "VertexArray.hpp"
#include "Texture.hpp"

#ifdef NDEBUG
extern const bool debug = false;
#else
extern const bool debug = true;
#endif

float const vertices[] = {
    //  100.0f,  100.0f,  0.0f,    0.0f, 0.0f, // 0
    //  200.0f,  100.0f,  0.0f,    1.0f, 0.0f, // 1
    //  200.0f,  200.0f,  0.0f,    1.0f, 1.0f, // 2
    //  100.0f,  200.0f,  0.0f,    0.0f, 1.0f  // 3
    -0.5f, -0.5f,  0.0f,    0.0f, 0.0f, // 0
     0.5f, -0.5f,  0.0f,    1.0f, 0.0f, // 1
     0.5f,  0.5f,  0.0f,    1.0f, 1.0f, // 2
    -0.5f,  0.5f,  0.0f,    0.0f, 1.0f  // 3

};
unsigned const indicies[] = {
    0, 1, 2,
    0, 2, 3
};

int main()
{
    Application app;
    Renderer renderer;
    Shader shader;
    VertexBuffer VB(vertices, sizeof(vertices));
    IndexBuffer IB(indicies, 6);
    VertexArray VA;
    VertexBufferlayout layout;
    Texture brickWallTexture("res/textures/wall.png");

    if(!shader.ParceShaderFile("src/basic.glsl")) return -1;
    if(!shader.CompileShaders()) return -1;

    layout.push<float>(3);
    layout.push<float>(2);
    shader.bind();
    brickWallTexture.bind();
    GLCALL(glUniform1i(shader.getUniform("u_Texture"), 0));
    VA.bind();  
    VA.addBuffer(VB, layout);

    // went insane with these matricies
    // glm::mat4 proj = glm::ortho(-320.0f, 320.0f, -240.0f, 240.0f, -1.0f, 1.0f);
    // glm::mat4 proj = glm::ortho(-640.0f, 640.0f, -480.0f, 480.0f, -1.0f, 1.0f);
    // glm::mat4 proj = glm::ortho(0.0f, 640.0f, 0.0f, 480.0f, -1.0f, 1.0f);
    glm::mat4 proj = glm::ortho(-2.0f, 2.0f, -1.5f, 1.5f, -1.0f, 1.0f);
    glm::mat4 view(1.0f);
    glm::mat4 model(1.0f);

    model = glm::translate(model, glm::vec3(0.75f, 0, 0));
    // model = glm::rotate(model, 1.0f, glm::vec3(0, 0, 45));

    view = glm::translate(view, glm::vec3(-1, 0, 0));
    // view = glm::rotate(view, 1.0f, glm::vec3(0, 0, 45));

    glm::mat4 MVP = proj * view * model;
    GLCALL(glUniformMatrix4fv(shader.getUniform("u_MVP"), 1, GL_FALSE, &MVP[0][0]));

    while (!glfwWindowShouldClose(window))
    {
        // frameBeginTimeSeconds = getTimeSeconds();

        renderer.Clear(0.05, 0.1, 0.12);

        // if(wireframe) {
        //     GLCALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        // } else {
        //     GLCALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        // }
        // GLCALL(glUniform4f(color1UniformLocation, color1[0], color1[1], color1[2], 0));
        // GLCALL(glUniform4f(color2UniformLocation, color2[0], color2[1], color2[2], 1));
        renderer.Draw(VA, IB, shader);
        // renderTimeSeconds = getTimeSeconds() - frameBeginTimeSeconds;

        // ImGui_ImplOpenGL3_NewFrame();
        // ImGui_ImplGlfw_NewFrame();
        // ImGui::NewFrame();

        // if(showDemoWindow) ImGui::ShowDemoWindow();

        // ImGui::Begin("debug");
        // ImGui::Text("FPS: %f", FPS);
        // ImGui::Text("renderFPS: %f", renderFPS);
        // ImGui::Text("delta time: %fms", displayDeltaTime * 1000);
        // ImGui::Text("render time: %fms", displayRenderTimeSeconds * 1000);
        // ImGui::InputInt("refresh rate", &refreshRate);
        // ImGui::End();

        // ImGui::Begin("properties");
        // ImGui::ColorEdit3("first color", color1);
        // ImGui::ColorEdit3("second color", color2);
        // ImGui::Checkbox("wireframe", &wireframe);
        // ImGui::Checkbox("demo window", &showDemoWindow);
        // if(ImGui::Combo("texture", &current_item, items, IM_ARRAYSIZE(items))) {
        //     switch (current_item) {
        //     case 0:
        //         brickWallTexture.bind();
        //         break;
        //     case 1:
        //         smileTexture.bind();
        //         break;
        //     default:
        //         brickWallTexture.unbind();
        //         break;
        //     }
        // }
        // ImGui::End();

        // ImGui::Render();
        // ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // GLFWwindow* backup_current_context = glfwGetCurrentContext();
        // ImGui::UpdatePlatformWindows();
        // ImGui::RenderPlatformWindowsDefault();
        // glfwMakeContextCurrent(backup_current_context);

        glfwSwapBuffers(window);
        glfwPollEvents();

        // deltaTime = getTimeSeconds() - frameBeginTimeSeconds;
        // if(iteration++ % (refreshRate > 0 ? refreshRate : 1) == 0) {
        //     FPS = deltaTime > 0 ? 1 / deltaTime : 0;
        //     renderFPS = renderTimeSeconds > 0 ? 1 / renderTimeSeconds : 0;
        //     displayRenderTimeSeconds = renderTimeSeconds;
        //     displayDeltaTime = deltaTime;
        //     glfwSetWindowTitle(window, (std::to_string((int) FPS) + " FPS").c_str());
        // }
    }
}
