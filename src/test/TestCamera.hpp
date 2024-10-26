#pragma once

#include "opengl/Renderer.hpp"
#include "opengl/GlCall.h"
#include "opengl/Shader.hpp"
#include "opengl/VertexBuffer.hpp"
#include "opengl/IndexBuffer.hpp"
#include "opengl/VertexArray.hpp"
#include "opengl/Texture.hpp"
#include "Camera.hpp"
#include "GLFW/glfw3.h"
#include "Test.hpp"

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

    class TestCamera : public Test
    {
    private:
        bool mouseLocked = true;
        bool firstCursorMove = true;
        float prevx = 0;
        float prevy = 0;
        float sensitivity = 0.05f;
        float cameraSpeed = 0.01f;

        Camera cam;
        Renderer renderer;
        Shader shader;
        VertexBuffer VB;
        IndexBuffer IB;
        VertexArray VA;
        VertexBufferlayout layout;
        Texture brickWallTexture;

        // in case you think thats all
        glm::vec3 translation1;
        glm::vec3 rotation1;
        glm::vec3 scale1;
        glm::vec3 translation2;
        glm::vec3 rotation2;
        glm::vec3 scale2;
        bool wireframe = false;
        int color1UniformLocation;
        float color[4] = {0, 0, 0, 1};
        bool showDemoWindow = false;
        const char* textureNames[2] = { "brick wall", "no texture"};
        int current_texture_item = 0; // Index to store the selected item
        bool show_another_window = false;
        bool object2 = false;
        
        // nope
        static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    public:
        ~TestCamera();
        TestCamera(GLFWwindow *window);
        void processCameraMovement(GLFWwindow *window, double deltatime);
        void onRender(GLFWwindow *window, double deltatime) override;
        void onImGuiRender(double deltatime) override;
    };
} // namespace test
