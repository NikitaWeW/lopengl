#pragma once
#include "opengl/Renderer.hpp"
#include "opengl/Shader.hpp"
#include "ControllableCamera.hpp"
#include "GLFW/glfw3.h"
#include "test/Test.hpp"

namespace test {
    class BasicScene : public test::Test
    {
    private:
    public:
        ControllableCamera camera;
        Renderer renderer;
        GLFWwindow *&window = camera.window;
        glm::vec4 clearColor = {0.05, 0.1, 0.12, 0};
        bool wireframe = false;

        BasicScene(GLFWwindow *window);
        virtual ~BasicScene();

        glm::mat4 getMVP(glm::mat4 model);
        void setMVPUniform(Shader &shader, glm::mat4 const &model, std::string const &name = "u_MVP");

        virtual void onRender(GLFWwindow *window, double deltatime) override;
        virtual void onImGuiRender(double deltatime) override;
        
        static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
        static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
    };
}