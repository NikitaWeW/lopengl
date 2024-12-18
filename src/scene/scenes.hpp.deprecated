#pragma once
#include "Scene.hpp"
#include "imgui.h"
#include "Application.hpp"
#include "glad/gl.h"

namespace scenes {
    class CubePartyScene : public Scene {
    public:
        void onRender(Application &app, Renderer &renderer, Camera &camera) override;
        void onImGuiRender(Application &app) override;
        inline char const * getName() const override { return "cube party"; }
    };
    class SingleModelScene : public Scene {
    public:
        virtual void onRender(Application &app, Renderer &renderer, Camera &camera) override;
        virtual void onImGuiRender(Application &app) override;
        virtual inline char const *getName() const override { return "single model"; }
    };
    class Blending : public Scene {
    private:
        Texture floorTexture  = Texture{"res/textures/sandy_gravel.jpg"}; 
        Texture grassTexture  = Texture{"res/textures/grass.png"}; 
        Shader lightingShader = Shader {"shaders/lighting.glsl", true};
    public: 
        int quality = 25;
        Blending();
        virtual void onRender(Application &app, Renderer &renderer, Camera &camera) override;
        virtual void onImGuiRender(Application &app) override;
        virtual inline char const *getName() const override { return "blending demo"; }
    };
}