#pragma once
#include <map>
#include <string>
#include "opengl/Texture.hpp"
#include "opengl/Shader.hpp"
#include "opengl/VertexBuffer.hpp"
#include "opengl/VertexArray.hpp"
#include "opengl/IndexBuffer.hpp"
#include "Test.hpp"

namespace test
{
    class TestTexture : public Test {
    private:
        std::vector<const char *> textureNames;
        std::map<std::string, Texture> textureCache;
        Texture *currentTexture;
        int currentTextureNameIndex;
        char filenamebuffer[1024];
        Shader shader;
        VertexBuffer VB;
        VertexArray  VA;
        IndexBuffer  IB;
        bool applyTexture(std::string const &filepath);
    public:
        TestTexture();
        ~TestTexture();
        void onRender(GLFWwindow *window, double deltatime) override;
        void onImGuiRender(double deltatime) override;
    };
}