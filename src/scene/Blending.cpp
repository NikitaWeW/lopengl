#include "scenes.hpp"

scenes::Blending::Blending()
{
}

glm::vec2 const grassPositions[] = {
    glm::vec2{  0.0f,  0.0f },
    glm::vec2{  2.0f,  5.0f },
    glm::vec2{ -1.5f, -2.2f },
    glm::vec2{ -3.8f, -2.0f },
    glm::vec2{  2.4f, -0.4f },
    glm::vec2{ -1.7f,  3.0f },
    glm::vec2{ -3.7f,  4.5f },
    glm::vec2{  7.1f, -8.2f },
    glm::vec2{ -6.3f,  9.4f },
    glm::vec2{  2.8f, -1.7f },
    glm::vec2{ -9.9f,  3.2f }
};
class comparator {
public:
    bool operator()(std::pair<float, float> const &left, std::pair<float, float> const &right) const {
        if (left.first != right.first) {
            return left.first < right.first;
        }
        return left.second < right.second;
    }
};

// unoptimized, poor, really bad code here.
void scenes::Blending::onRender(Application &app, Renderer &renderer, Camera &camera)
{
    renderer.clear(app.clearColor);

    app.quad.resetMatrix();
    app.quad.translate({1, -1, -1});
    app.quad.rotate({90, 0, 0});
    app.quad.scale({100, 100, 1});

    floorTexture.bind();
    renderer.draw(app.quad, app.shaders[app.currentShaderIndex], camera); // draw floor

    std::map<std::pair<float, float>, std::pair<glm::vec3, glm::vec3>, comparator> sorted;
    for(glm::vec2 const &position : grassPositions) { // generate grass bushes and sort them so 

        for(int i = 0; i < quality; ++i) {
            app.quad.m_position = {position.x, 0, position.y};
            app.quad.m_rotation = {std::sin(i) * 10, i * 360 / quality, std::sin(i) * 10};
            app.quad.rotate();
            sorted[
                std::make_pair(
                    glm::distance(camera.position, app.quad.m_position),
                    1 - glm::dot(camera.position - app.quad.m_position, glm::vec3{glm::vec4{1, 0, 1, 1} * app.quad.getModelMat()})
                )
            ] = std::make_pair(
                app.quad.m_position, 
                app.quad.m_rotation);
        }
    }

    grassTexture.bind();
    for(std::map<std::pair<float, float>, std::pair<glm::vec3, glm::vec3>, comparator>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
        app.quad.resetMatrix();
        app.quad.translate(it->second.first);
        app.quad.rotate(   it->second.second);

        renderer.draw(app.quad, app.shaders[app.currentShaderIndex], camera);
    }
}

void scenes::Blending::onImGuiRender(Application &app)
{
    ImGui::Text("its broken rn... maybe ill fix it one day");
    ImGui::InputInt("grass intencity", &quality);
    quality = std::max(1, quality);
}
