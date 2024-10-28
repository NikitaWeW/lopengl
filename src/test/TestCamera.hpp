#pragma once

#include "opengl/Renderer.hpp"
#include "opengl/GlCall.h"
#include "opengl/Shader.hpp"
#include "opengl/VertexBuffer.hpp"
#include "opengl/IndexBuffer.hpp"
#include "opengl/VertexArray.hpp"
#include "opengl/Texture.hpp"
#include "ControllableCamera.hpp"
#include "GLFW/glfw3.h"
#include "BasicScene.hpp"

namespace test
{
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

    class TestCamera : public BasicScene
    {
    private:
        Shader shader;
        VertexBuffer VB;
        IndexBuffer IB;
        VertexArray VA;
        VertexBufferlayout layout;
        Texture brickWallTexture;

        glm::vec3 translation1 = glm::vec3(0);
        glm::vec3 rotation1 = glm::vec3(0);
        glm::vec3 scale1 = glm::vec3(1);
        glm::vec3 translation2 = glm::vec3(2.5, 1, -3);
        glm::vec3 rotation2 = glm::vec3(0, 90, 0);
        glm::vec3 scale2 = glm::vec3(1);
        bool wireframe = false;
        int color1UniformLocation;
        float color[4] = {0, 0, 0, 1};
        bool showDemoWindow = false;
        const char* textureNames[2] = { "brick wall", "no texture"};
        int current_texture_item = 0; // Index to store the selected item
        bool show_another_window = false;
        bool object2 = false;
        
        static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
        static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

    public:
        ~TestCamera();
        TestCamera(GLFWwindow *window);
        void onRender(GLFWwindow *window, double deltatime) override;
        void onImGuiRender(double deltatime) override;
    };
} // namespace test
