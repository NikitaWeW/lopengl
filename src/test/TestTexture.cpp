#include "TestTexture.hpp"
#include "opengl/GlCall.h"
#include "imgui.h"
#include <stdexcept>

test::TestTexture::TestTexture() :
    VB(test::squareVertices, sizeof(test::squareVertices)),
    IB(test::squareIndicies, sizeof(squareIndicies))
{
    LOG_INFO("creating test");
    if(!shader.ParceShaderFile("src/basic.glsl")) throw std::runtime_error("failed to parce shaders!");
    if(!shader.CompileShaders()) throw std::runtime_error("failed to compile shaders!");
    currentTexture = &Texture{"res/textures/smile.png"};
    VertexBufferlayout layout;
    layout.push<float>(3);
    layout.push<float>(2);
    shader.bind();
    GLCALL(glUniform1i(shader.getUniform("u_Texture"), 0));
    VA.bind();  
    VA.addBuffer(VB, layout);
}

test::TestTexture::~TestTexture() {
    LOG_INFO("destroying test");
}

void test::TestTexture::onRender(GLFWwindow *window, double deltatime) {
    currentTexture->bind();
}

void test::TestTexture::onImGuiRender(double deltatime) {
    ImGui::Begin("texture test");
    if(ImGui::Combo("textures", &currentTextureNameIndex, textureNames.data(), (int) textureNames.size())) {
        applyTexture(textureNames.at(currentTextureNameIndex));
    };
    ImGui::InputText("file path", filenamebuffer, sizeof(filenamebuffer));
    if(ImGui::Button("open")) {
        applyTexture(filenamebuffer);
    };
    ImGui::End();
}
bool test::TestTexture::applyTexture(std::string const &filepath) {
    if(textureCache.find(filepath) != textureCache.end()) {
        textureCache.at(filepath).bind();
        return true;
    }
    Texture newTexture{filepath};
    currentTexture = &newTexture;
    textureCache[filepath] = newTexture;
    textureNames.push_back(filepath.c_str());
}