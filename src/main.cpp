#include <bits/stdc++.h>

#include "Core/Resource/Loaders.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "Core/Controller.hpp"
#include "Core/ogl.hpp"
#include "Core/ModelMat.hpp"
#include "Core/Logging.hpp"
#include "Core/IO.hpp"

bool init(GLFWwindow **window);

static std::string printTexture(Entity e)
{
    if(!e.valid())
        return fmt::format("e{} -- INVALID", e.entity());
    auto const &texture = e.get<Texture>();
    return fmt::format("e{}, \"{:<30} {}x{}, {:>3}", e.entity(), texture.path + "\",", texture.bitmap.getWidth(), texture.bitmap.getHeight(),(texture.srgb ? "srgb" : "not srgb"));
}
static void printModelData(Entity e)
{
    assert(reg.has<Model>(e));
    Model const &model = e.get<Model>();
    LOG_INFO("");
    LOG_INFO("Model: e{}: \"{}\"", e.entity(), model.path);
    LOG_INFO("Skeleton: ");
    LOG_INFO("  Bone map size / number of bones: {}", model.skeleton.boneMap.size());
    if(model.skeleton.boneMap.size() <= 30)
        for(auto const &[name, id] : model.skeleton.boneMap)
            LOG_INFO("    [\"{}\": {}]", name, id);

    LOG_INFO("Animations: {}", model.animations.size());
    for(auto const &animation : model.animations)
    {
        LOG_INFO("-----------------");
        LOG_INFO("Animation: \"{}\"", animation.name);
        LOG_INFO("  Duration: {} ticks, tps: {}", animation.durationTicks, animation.ticksPerSecond);
        LOG_INFO("  Bones size: {}", animation.bones.size());
    }

    LOG_INFO("Meshes: {}", model.meshes.size());
    for(auto const &mesh : model.meshes)
    {
        LOG_INFO("-----------------");

        LOG_INFO("Geometry:");
        LOG_INFO("  Triangles: {}", mesh.geometry.indices.size() / 3);
        LOG_INFO("  Indices:   {}", mesh.geometry.indices.size());
        LOG_INFO("  Positions: {}", mesh.geometry.positions.size());
        LOG_INFO("  TexCoords: {}", mesh.geometry.texCoords.size());
        LOG_INFO("  Normals:   {}", mesh.geometry.normals.size());
        LOG_INFO("  Tangents:  {}", mesh.geometry.tangents.size());
        LOG_INFO("  BoneIDs:   {}", mesh.geometry.boneIDs.size());
        LOG_INFO("  Weights:   {}", mesh.geometry.weights.size());
        
        LOG_INFO("Material:");
        LOG_INFO("Textures:");
        LOG_INFO("  Albedo:       {}", printTexture({&e.reg(), mesh.material.textures.albedo}));
        LOG_INFO("  Metallic:     {}", printTexture({&e.reg(), mesh.material.textures.metallic}));
        LOG_INFO("  Roughness:    {}", printTexture({&e.reg(), mesh.material.textures.roughness}));
        LOG_INFO("  Ambient:      {}", printTexture({&e.reg(), mesh.material.textures.ambient}));
        LOG_INFO("  Normal:       {}", printTexture({&e.reg(), mesh.material.textures.normal}));
        LOG_INFO("  Displacement: {}", printTexture({&e.reg(), mesh.material.textures.displacement}));
        LOG_INFO("Properties:");
        LOG_INFO("  Ambient:       {}", fmt::streamed(mesh.material.properties.ambient));
        LOG_INFO("  Albedo:        {}", fmt::streamed(mesh.material.properties.albedo));
        LOG_INFO("  Specular:      {}", fmt::streamed(mesh.material.properties.specular));
        LOG_INFO("  Emission:      {}", fmt::streamed(mesh.material.properties.emission));
        LOG_INFO("  Shininess:     {}", mesh.material.properties.shininess);
        LOG_INFO("  Metallic:      {}", mesh.material.properties.metallic);
        LOG_INFO("  IOR:           {}", mesh.material.properties.ior);
    }
}

static Entity loadModel(std::string_view path, ModelLoaderOptions options = {}, std::optional<Material> material = {})
{
    static ModelLoader loader(sReg.getReg());
    
    auto eModel = Entity{&sReg, loader.loadFromFile(path, options)};
    auto &model = eModel.get<Model>();
    
    if(material.has_value())
    {
        auto defaultMaterial = loader.getDefaultMaterial();
        if(material->textures.albedo       == INVALID_ENTITY) material->textures.albedo       = defaultMaterial.textures.albedo;
        if(material->textures.metallic     == INVALID_ENTITY) material->textures.metallic     = defaultMaterial.textures.metallic;
        if(material->textures.roughness    == INVALID_ENTITY) material->textures.roughness    = defaultMaterial.textures.roughness;
        if(material->textures.ambient      == INVALID_ENTITY) material->textures.ambient      = defaultMaterial.textures.ambient;
        if(material->textures.normal       == INVALID_ENTITY) material->textures.normal       = defaultMaterial.textures.normal;
        if(material->textures.displacement == INVALID_ENTITY) material->textures.displacement = defaultMaterial.textures.displacement;
        eModel.reg()->get<Texture>(material->textures.albedo).srgb = true;

        for(auto &mesh : model.meshes)
            mesh.material = material.value();
    }

    printModelData(eModel);

    return eModel;
}
struct OglModel
{
    struct OglMesh
    {
        struct Material {
            struct Textures {
                ogl::Texture albedo;
                ogl::Texture metallic;
                ogl::Texture roughness;
                ogl::Texture ambient;
                ogl::Texture normal;
                ogl::Texture displacement;
            } textures;
            ::Material::Properties properties;
        } material;

        ogl::VAO vao;
        ogl::IBO ibo;
        GLenum mode = GL_TRIANGLES;

        unsigned count = 0;
    };

    std::vector<OglMesh> meshes;
    std::vector<Animation> animations;
    std::string path;
    Entity eSource;

    ::Model::Skeleton skeleton;
};
static ogl::Texture processTexture(Entity eTexture)
{
    if(!eTexture.has<ogl::Texture>())
    {
        auto &texture = eTexture.get<Texture>();
        eTexture.emplace<ogl::Texture>(ogl::makeTexture(texture.bitmap, texture.srgb));
    }
    return eTexture.get<ogl::Texture>();
}
static OglModel &allocate(Entity eModel)
{
    if(!eModel.has<OglModel>())
    {
        assert(eModel.has<Model>());
        Model model = eModel.get<Model>();
        
        OglModel oglModel{
            .animations = model.animations,
            .path = model.path,
            .eSource = eModel,
            .skeleton = model.skeleton
        };
    
        for(auto const &mesh : model.meshes)
        {
            OglModel::OglMesh oglMesh;
            oglMesh.mode = GL_TRIANGLES;
            oglMesh.count = mesh.geometry.indices.size();
    
            oglMesh.material = {
                .textures = {
                    .albedo       = processTexture({&sReg, mesh.material.textures.albedo      }),
                    .metallic     = processTexture({&sReg, mesh.material.textures.metallic    }),
                    .roughness    = processTexture({&sReg, mesh.material.textures.roughness   }),
                    .ambient      = processTexture({&sReg, mesh.material.textures.ambient     }),
                    .normal       = processTexture({&sReg, mesh.material.textures.normal      }),
                    .displacement = processTexture({&sReg, mesh.material.textures.displacement}),
                },
                .properties = oglMesh.material.properties
            };
    
            glCreateVertexArrays(1, &oglMesh.vao.id);
            
            ogl::pushVertexBuffer(oglMesh.vao, ogl::makeBuffer<ogl::VBO>(mesh.geometry.positions), 3, GL_FLOAT);
            ogl::pushVertexBuffer(oglMesh.vao, ogl::makeBuffer<ogl::VBO>(mesh.geometry.texCoords), 2, GL_FLOAT);
            ogl::pushVertexBuffer(oglMesh.vao, ogl::makeBuffer<ogl::VBO>(mesh.geometry.normals),   3, GL_FLOAT);
            ogl::pushVertexBuffer(oglMesh.vao, ogl::makeBuffer<ogl::VBO>(mesh.geometry.tangents),  3, GL_FLOAT);
            ogl::pushVertexBuffer(oglMesh.vao, ogl::makeBuffer<ogl::VBO>(mesh.geometry.boneIDs),   3, GL_FLOAT);
            ogl::pushVertexBuffer(oglMesh.vao, ogl::makeBuffer<ogl::VBO>(mesh.geometry.weights),   3, GL_FLOAT);
            
            oglMesh.ibo = ogl::makeBuffer<ogl::IBO>(mesh.geometry.indices);
            glVertexArrayElementBuffer(oglMesh.vao.id, oglMesh.ibo.id);
    
            oglModel.meshes.emplace_back(std::move(oglMesh));
        }
    
        eModel.emplace<OglModel>(oglModel);
    }

    return eModel.get<OglModel>();
}

int main(int argc, char **argv) {
    GLFWwindow* window;
    if(!init(&window)) {
        LOG_ERROR("Failed to init!");
        return -1;
    }

    Controller controller{};
    auto &camera = controller.createCamera(sReg, {0, 3, 3}).get<Controller::Camera>();
    TextureLoader textureLoader{sReg.getReg()};
    
    auto const &cube = allocate(loadModel("assets/suzanne.glb", {}, Material{
        .textures = {
            .albedo = textureLoader.loadFromFile("assets/wood.jpg")
        }
    }));
    ogl::Program shader = ogl::compileShader("shaders/atmosphere");
    ogl::Program gridShader = ogl::compileShader("shaders/grid");

    if(!shader.id || !gridShader.id)
    {
        LOG_ERROR("Failed to compile shaders!");
        return -1;
    }
    
    glm::vec3 rotation{0};
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glDisable(GL_CULL_FACE);

    auto &listener = sReg.create<EventListener>().get<EventListener>();
    
    glm::ivec2 size{0};
    float deltatime = 0.1;
    glm::vec3 sunPos(10, 3, 10);
    while (!glfwWindowShouldClose(window))
    {
        auto start = std::chrono::high_resolution_clock::now();
        controller.update(sReg, deltatime);
        glfwGetWindowSize(window, &size.x, &size.y);

        while(!listener.keyEvents.empty())
        {
            auto event = listener.keyEvents.front();
            listener.keyEvents.pop();

            if(event.key == GLFW_KEY_R && event.action == GLFW_PRESS)
            {
                LOG_INFO("Recompiling \"{}\"", shader.dirpath);
                auto newShader = ogl::compileShader(shader.dirpath);
                if(newShader.id)
                {
                    shader = newShader;
                }
            }
        }
        while(!listener.mouseButtonEvents.empty())
        {
            auto event = listener.mouseButtonEvents.front();
            listener.mouseButtonEvents.pop();

            if(event.button == GLFW_MOUSE_BUTTON_LEFT && event.action == GLFW_PRESS)
            {
                glm::vec3 forward = glm::mat3(glm::inverse(camera.viewMat)) * glm::vec3{0, 0,-1};
                sunPos = forward * 10.0f;
                LOG_VAR(sunPos);
            }
        }

        for(auto eWindow : sReg.view<Window>())
            eWindow.get<Window>().size = size;


        glViewport(0, 0, size.x, size.y);
        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glUseProgram(shader.id);
        glUniformMatrix4fv(ogl::getUniform(shader, "uViewMat"), 1, GL_FALSE, glm::value_ptr(camera.viewMat));
        glUniformMatrix4fv(ogl::getUniform(shader, "uProjMat"), 1, GL_FALSE, glm::value_ptr(camera.projMat));
        
        glUniform3fv(ogl::getUniform(shader, "uLightPos"), 1, glm::value_ptr(sunPos));
        glDepthFunc(GL_LEQUAL);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 14);
        glDepthFunc(GL_LESS);

        glUseProgram(gridShader.id);
        glUniformMatrix4fv(ogl::getUniform(gridShader, "uViewMat"), 1, GL_FALSE, glm::value_ptr(camera.viewMat));
        glUniformMatrix4fv(ogl::getUniform(gridShader, "uProjMat"), 1, GL_FALSE, glm::value_ptr(camera.projMat));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        deltatime = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count()) * 1.0E-6;
    }

    glfwTerminate();
}
