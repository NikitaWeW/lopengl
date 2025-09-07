#include "main.hpp"

static bool shouldProcessInput(Data &)
{
    return !ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);
    // return !ImGui::GetIO().WantCaptureMouse;
}

void updateVP(Data &data, bool cameraLocked)
{
    data.yawPitch.update(data.deltatime);
    data.yawPitch.falloff = glm::mix(glm::vec2{10.0f}, glm::vec2{5.0f}, static_cast<float>(!cameraLocked));
    data.distance.update(data.deltatime);
    data.distance.value = glm::clamp<float>(data.distance.value, 1, 5);

    if(glm::abs(data.yawPitch.value.y) > 360.0f)
    {
        data.yawPitch.value.y = glm::mod(data.yawPitch.value.y, 360.0f);
    }

    data.viewMat = glm::mat4{1.0f};
    data.viewMat = glm::translate(
        data.viewMat,
        glm::vec3{0, 0, -data.distance.value}
    );
    data.viewMat = glm::rotate(
        data.viewMat,
        glm::radians(data.yawPitch.value.y),
        glm::vec3{1, 0, 0}
    );
    data.viewMat = glm::rotate(
        data.viewMat,
        glm::radians(data.yawPitch.value.x),
        glm::vec3{0, 1, 0}
    );
    data.projMat = glm::perspective<float>(glm::radians(45.0f), (float) data.windowSize.x / data.windowSize.y, 0.01, 100);

    data.cameraPos = glm::inverse(data.viewMat) * glm::vec4{0, 0, 0, 1};
    data.cameraDir = glm::inverse(data.viewMat) * glm::vec4{0, 0,-1, 0};
}
void processInput(Data &data)
{
    assert(data.window);

    bool cameraLocked = glfwGetMouseButton(data.window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    glfwSetInputMode(data.window, GLFW_CURSOR, cameraLocked ? GLFW_CURSOR_CAPTURED : GLFW_CURSOR_NORMAL);
    
    bool focused = shouldProcessInput(data);
    
    glfwGetCursorPos(data.window, &data.mousePos.x, &data.mousePos.y);
    data.deltaMouse = data.prevMousePos - data.mousePos;
    data.deltaMouse.x = -data.deltaMouse.x;
    data.deltaMouse /= glm::max<float>(data.windowSize.x, data.windowSize.y) / 1000.0f;
    data.prevMousePos = data.mousePos;

    glm::vec2 vel{0};

    if((glfwGetKey(data.window, GLFW_KEY_LEFT)  == GLFW_PRESS) && focused) vel.x += data.deltatime * data.inputs.sensitivity * 400.0f;
    if((glfwGetKey(data.window, GLFW_KEY_RIGHT) == GLFW_PRESS) && focused) vel.x -= data.deltatime * data.inputs.sensitivity * 400.0f;
    if((glfwGetKey(data.window, GLFW_KEY_UP)    == GLFW_PRESS) && focused) vel.y += data.deltatime * data.inputs.sensitivity * 400.0f;
    if((glfwGetKey(data.window, GLFW_KEY_DOWN)  == GLFW_PRESS) && focused) vel.y -= data.deltatime * data.inputs.sensitivity * 400.0f;
    if(cameraLocked && focused) 
    {
        vel += glm::vec2{data.deltaMouse.x, -data.deltaMouse.y} * data.deltatime * data.inputs.sensitivity * 400.0f;
    }

    if(glm::abs(data.yawPitch.value.y) > 90 && glm::abs(data.yawPitch.value.y) <= 270)
        vel.x = -vel.x;

    data.yawPitch.velocity += vel;

    updateVP(data, cameraLocked);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    Data &data = *static_cast<Data *>(glfwGetWindowUserPointer(window));
    if(shouldProcessInput(data)) {
        data.distance.velocity -= yoffset * data.deltatime * data.inputs.sensitivity * 200.0f;
    } else {
        ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    }
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Data &data = *static_cast<Data *>(glfwGetWindowUserPointer(window));
    if(shouldProcessInput(data)) 
    {
        if(key == GLFW_KEY_R && action == GLFW_PRESS)
        {
            rebuildShaders(data);
            generateTexture(data);
        }
    } else 
    {
        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    }
}
void randomSeed(Data &data)
{
    data.inputs.seed = data.dist(data.gen);
    generateTexture(data);
}
