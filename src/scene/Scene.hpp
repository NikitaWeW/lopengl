/*
    base scene only. for scenes see src/scene/scenes.hpp
*/
#pragma once
#include <string>
#include "opengl/Renderer.hpp"
struct Application;

class Scene {
public:
    virtual ~Scene() {}
    virtual void onRender(Application &app, Renderer &renderer, Camera &camera) {}
    virtual void onImGuiRender(Application &app) {}
    inline virtual char const * getName() const { return "unnamed"; }
};
