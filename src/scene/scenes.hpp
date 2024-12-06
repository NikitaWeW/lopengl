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
}