#pragma once

#include "ogl.hpp"
#include "glm/glm.hpp"

namespace texture
{
    ogl::Texture load(std::string_view path, std::string_view type);
} // namespace texture

namespace model
{
    struct Material
    {
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
        glm::vec3 transmittance;
        glm::vec3 emission;

        float shininess;
        float ior;
    };
    struct MaterialTextures
    {
        ogl::Texture ambient;
        ogl::Texture diffuse;
        ogl::Texture specular;
        ogl::Texture bump;
        ogl::Texture displacement;
        ogl::Texture alpha;
        ogl::Texture reflection;
    };
    struct Mesh
    {
        struct VertexBuffers
        {
            ogl::VBO positions;
            ogl::VBO texCoords;
            ogl::VBO normals;
            ogl::VBO tangents;
        } buffers;

        Material material;
        MaterialTextures textures;
        ogl::VAO vao;
        unsigned count = 0;
    };
    
    class ILoader
    {
    public:
        virtual ~ILoader() = default;
        virtual Mesh load(std::string_view path) = 0;
    };
    
    class Loader
    {
    private:
        std::unordered_map<std::string, std::unique_ptr<ILoader>> m_loaders;
    public:
        Loader();
        void registerLoader(std::string_view extension, std::unique_ptr<ILoader> &&loader);
        Mesh load(std::string_view path);
    };
    
    class ObjLoader : public ILoader
    {
    public:
        Mesh load(std::string_view path) final;
    };
    class GltfLoader : public ILoader
    {
    public:
        Mesh load(std::string_view path) final;
    };
} // namespace model

