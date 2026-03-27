#pragma once

#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include <queue>
#include "DSA/ECS.hpp"

struct Window
{
    glm::uvec2 size;
    GLFWwindow *handle;
};
struct EventListener
{
    struct KeyEvent
    {
        GLFWwindow *window;
        int key;
        int scancode;
        int action;
        int mods;
    };
    struct CursorPosEvent
    {
        GLFWwindow *window;
        glm::dvec2 pos;
        glm::dvec2 delta;
    };
    struct ScrollEvent
    {
        GLFWwindow *window;
        glm::dvec2 offset;
    };
    std::queue<KeyEvent> keyEvents;
    std::queue<CursorPosEvent> cursorPosEvents;
    std::queue<ScrollEvent> scrollEvents;

    glm::dvec2 prevCursorPos{-1};
};

extern Registry sReg;

inline void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Registry &reg = *static_cast<Registry *>(glfwGetWindowUserPointer(window));
    for(auto e : reg.view<EventListener>())
        e.get<EventListener>().keyEvents.emplace(window, key, scancode, action, mods);
}
inline void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    Registry &reg = *static_cast<Registry *>(glfwGetWindowUserPointer(window));
    auto cursorPos = glm::dvec2{xpos, ypos};
    for(auto e : reg.view<EventListener>())
    {
        glm::dvec2 delta{0};
        auto &listener = e.get<EventListener>();
        if(listener.prevCursorPos != glm::dvec2{-1})
            delta = cursorPos - listener.prevCursorPos;
        listener.prevCursorPos = cursorPos;
        listener.cursorPosEvents.emplace(window, cursorPos, delta);
    }
}
inline void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    Registry &reg = *static_cast<Registry *>(glfwGetWindowUserPointer(window));
    for(auto e : reg.view<EventListener>())
        e.get<EventListener>().scrollEvents.emplace(window, glm::dvec2{xoffset, yoffset});
}